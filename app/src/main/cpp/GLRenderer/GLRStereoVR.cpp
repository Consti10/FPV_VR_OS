
#include "GLRStereoVR.h"
#include "jni.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTexture.h>
#include <TelemetryReceiver.h>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <android/choreographer.h>
#include <MatrixHelper.h>
#include <CardboardViewportOcclusion.hpp>
#include <AndroidThreadPrioValues.hpp>
#include <NDKThreadHelper.hpp>
#include <Extensions.h>
#include <android/trace.h>
#include <TexturedGeometry.hpp>

GLRStereoVR::GLRStereoVR(JNIEnv* env, jobject androidContext, TelemetryReceiver& telemetryReceiver, gvr_context *gvr_context, const int videoMode, jlong vsyncP):
        mSurfaceTextureUpdate(env),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
        videoMode(static_cast<VideoModesHelper::VIDEO_RENDERING_MODE>(videoMode)), mSettingsVR(env, androidContext),
        mTelemetryReceiver(telemetryReceiver),
        vrCompositorRenderer(env,androidContext,gvr_api_.get(),mSettingsVR.isVR_DISTORTION_CORRECTION_ENABLED(),false),
        OSD_RATIO(SettingsOSDStyle(env,androidContext,0).OSD_STEREO_RATIO)
        {
    mFBRManager=std::make_unique<FBRManager>(VSYNC::createFrom(vsyncP),false);
    MLOGD<<"OSD_RATIO "<<OSD_RATIO;
}

void GLRStereoVR::placeGLElements(){
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    //The video width defaults to 10(cm). Calculate the z value such that the video fills a FOV
    //of exactly DEFAULT_FOV_FILLED_BY_SCENE
    float videoZ=-videoW/2.0f/glm::tan(glm::radians(VRSettings::DEFAULT_FOV_FILLED_BY_SCENE/2.0f));
    videoZ*=1/(mSettingsVR.VR_SCENE_SCALE_PERCENTAGE/100.0f);
    //
    vrCompositorRenderer.removeLayers();
    const unsigned int TESSELATION_FACTOR=10;
    const auto headTrackingMode=mSettingsVR.isHeadTrackingEnabled() ? VrCompositorRenderer::FULL:VrCompositorRenderer::NONE;

    const auto vid1=VideoModesHelper::createMeshForMode(videoMode,videoZ,videoW,videoH);
    vrCompositorRenderer.addLayer(vid1,&mSurfaceTextureUpdate,headTrackingMode);

    const auto osd=TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,videoZ},{videoW,videoW*1.0f/OSD_RATIO},0.0f,1.0f,false,false);
    const auto headTrackingModeForOSD=mSettingsVR.GHT_OSD_FIXED_TO_HEAD ? VrCompositorRenderer::NONE : headTrackingMode;
    vrCompositorRenderer.addLayer(osd, &osdRenderbuffer,headTrackingModeForOSD);
}

void GLRStereoVR::onContextCreated(JNIEnv * env, jobject androidContext, int screenW, int screenH, jobject surfaceTextureHolder) {
    Extensions::initializeGL();
    NDKThreadHelper::setProcessThreadPriority(env,FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_STEREO,__CLASS_NAME__.c_str());
    vrCompositorRenderer.initializeGL();
    //Once we have an OpenGL context, we can create our OpenGL world object instances.
    mSurfaceTextureUpdate.updateFromSurfaceTextureHolder(env,surfaceTextureHolder);
    //
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glClearColor(0,0,0,0.0F);
    placeGLElements();
}

// When we have VSYNC disabled ( which always means rendering into the front buffer directly) onDrawFrame is called as fast as possible.
// To not waste too much CPU & GPU on frames where the video did not change I limit the OpenGL FPS to max. 60fps here, but
// instead of sleeping I poll on the surfaceTexture in small intervalls to see if a new frame is available
// As soon as a new video frame is available, I render the OpenGL frame immediately
// This is not as efficient as using a condition variable but since the callback is invoked in java it might be hard to implement that
// TODO currently depreacted. I cannot account for all the different video fps combined with device GPU performance
void GLRStereoVR::waitUntilVideoFrameAvailable(JNIEnv* env, const std::chrono::steady_clock::time_point& maxWaitTimePoint) {
    if(const auto delay=mSurfaceTextureUpdate.waitUntilFrameAvailable(env,maxWaitTimePoint)){
        surfaceTextureDelay.add(*delay);
        //MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
    }else{
        //MLOGD<<"Timeout";
        ATrace_beginSection("Timeout");
        ATrace_endSection();
    }
    MLOGD<<"Delay of SurfaceTexture"<<surfaceTextureDelay.getAvgReadable();
}

void GLRStereoVR::calculateFrameTimes() {
    // remove frames we are done with
    while(!mPendingFrames.empty()){
        const auto& submittedFrame=mPendingFrames.front();
        auto stats=FrameTimestamps::getFrameTimestamps(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), submittedFrame.frameId);
        if(stats){
            //FrameTimestamps:logStats(submittedFrame.creationTime,*stats);
            MLOGD<<"To present "<<MyTimeHelper::R(std::chrono::nanoseconds(stats->DISPLAY_PRESENT_TIME_ANDROID-submittedFrame.creationTime.time_since_epoch().count()));
            mPendingFrames.pop();
        }else{
            break;
        }
    }
    auto thisFrame=FrameTimestamps::getNextFrameId(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
    if(thisFrame){
        if(mPendingFrames.size()>4){
            mPendingFrames.pop();
        }
        mPendingFrames.push(FrameTimestamps::SubmittedFrame{std::chrono::steady_clock::now(), *thisFrame});
    }
    //MLOGD<<"Frames in queue "<<mPendingFrames.size();
    //Extensions2::GetCompositorTimingANDROID(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));3
}

void GLRStereoVR::beforeDrawFrame(JNIEnv *env) {
    if(checkAndResetVideoFormatChanged()){
        placeGLElements();
    }
    mVrFTCalculator.tick();
    if(mSettingsVR.VR_RENDERING_MODE==3){
        mTelemetryReceiver.setOpenGLFPS(-1);
    }else{
        mTelemetryReceiver.setOpenGLFPS(mVrFTCalculator.getCurrentFPS());
    }
    vrCompositorRenderer.updateLatestHeadSpaceFromStartSpaceRotation();
}

void GLRStereoVR::onDrawFrame(JNIEnv* env) {
    this->currEnv=env;
    ATrace_beginSection("GLRStereoVR::onDrawFrame");
    beforeDrawFrame(env);
    //
    if(mSettingsVR.VR_RENDERING_MODE==0 || mSettingsVR.VR_RENDERING_MODE==1){
        //
        ATrace_beginSection("My updateVideoFrame");
        mSurfaceTextureUpdate.updateAndCheck(env);
        ATrace_endSection();
        //
        ATrace_beginSection("MglClear");
        GLHelper::updateSetClearColor(swapColor);
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        ATrace_endSection();
        for(int eye=0;eye<2;eye++){
            vrCompositorRenderer.drawLayers(eye==0 ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
        }
        //calculateFrameTimes();
        //EGLint rects[]={0,0,1080,1080};
        //Extensions::eglSwapBuffersWithDamageKHR(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW),rects,1);
        //eglSwapBuffers(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
    }
    if(mSettingsVR.VR_RENDERING_MODE==2){
        mFBRManager->drawEyesToFrontBufferUnsynchronized(env,vrCompositorRenderer);
    }else if(mSettingsVR.VR_RENDERING_MODE==3){
        mFBRManager->enterWarping(env,vrCompositorRenderer,[this](JNIEnv* env) {
            beforeDrawFrame(env);
        });
    }
    ATrace_endSection();
    //eglSwapBuffers(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
}


void GLRStereoVR::onSecondaryContextCreated(JNIEnv* env, jobject androidContext) {
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,mTelemetryReceiver,true,RENDER_TEX_W,RENDER_TEX_H);
    osdRenderbuffer.initializeGL();
    osdRenderbuffer.setSize(RENDER_TEX_W,RENDER_TEX_H);
    //auto framebuffer_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
    //MLOGD<<"W "<<framebuffer_size.width<<"H "<<framebuffer_size.height;
}


void GLRStereoVR::onSecondaryContextDoWork(JNIEnv* env) {
    mOSDFTCalculator.tick();
    mOSDFTLimiter.tick();
    //MLOGD<<"OSD fps"<<mOSDFPSCalculator.getCurrentFPS();
    ATrace_beginSection("GLRStereoVR::onSecondaryContextDoWork");
    osdRenderbuffer.bind();
    //TimerQuery timerQuery;
    //timerQuery.begin();
    glClearColor(1,0,0,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    mOSDRenderer->updateAndDrawElementsGL();
    ATrace_beginSection("GLRStereoVR::onSecondaryContextDoWork-GPU");
    osdRenderbuffer.unbindAndSwap();
    //timerQuery.end();
    //timerQuery.print();
    ATrace_endSection();
    ATrace_endSection();
}



//----------------------------------------------------JAVA bindings---------------------------------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_play_1stereo_GLRStereoVR_##method_name

inline jlong jptr(GLRStereoVR *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRStereoVR *native(jlong ptr) {
    return reinterpret_cast<GLRStereoVR *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jlong telemetryReceiver, jlong native_gvr_api,jint videoMode,jlong vsync) {
        return jptr(
            new GLRStereoVR(env, androidContext, *reinterpret_cast<TelemetryReceiver*>(telemetryReceiver), reinterpret_cast<gvr_context *>(native_gvr_api), videoMode, vsync));
}

JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject instance, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnContextCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject androidContext,jint screenW,jint screenH,jobject surfaceTextureHolder) {
    native(glRendererStereo)->onContextCreated(env,androidContext,screenW,screenH,surfaceTextureHolder);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    native(glRendererStereo)->onDrawFrame(env);
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->SetVideoRatio((int)videoW,(int)videoH);
}

JNI_METHOD(void, nativeOnSecondaryContextCreated)
(JNIEnv *env, jobject obj,jlong p,jobject context) {
    native(p)->onSecondaryContextCreated(env,context);
}
JNI_METHOD(void, nativeOnSecondaryContextDoWork)
(JNIEnv *env, jobject obj,jlong p) {
    native(p)->onSecondaryContextDoWork(env);
}

}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079