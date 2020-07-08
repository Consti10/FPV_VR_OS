
#include "GLRStereoNormal.h"
#include "jni.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTexture.h>
#include <TelemetryReceiver.h>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

//#define CHANGE_SWAP_COLOR
constexpr auto TAG= "GLRendererStereo";

#include <android/choreographer.h>
#include <MatrixHelper.h>
#include <CardboardViewportOcclusion.hpp>
#include <AndroidThreadPrioValues.hpp>
#include <NDKThreadHelper.hpp>
#include <Extensions.hpp>
#include <android/trace.h>
#include <TexturedGeometry.hpp>

//#define CHANGE_SWAP_COLOR

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,const int videoMode):
        mSurfaceTextureUpdate(env),
        gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
        videoMode(static_cast<VideoModesHelper::VIDEO_RENDERING_MODE>(videoMode)), mSettingsVR(env, androidContext),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"),
        vrCompositorRenderer(gvr_api_.get(),mSettingsVR.VR_DISTORTION_CORRECTION_MODE != 0){
}

void GLRStereoNormal::placeGLElements(){
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    //The video width defaults to 10(cm). Calculate tze z value such that the video fills a FOV
    //of exactly DEFAULT_FOV_FILLED_BY_SCENE
    float videoZ=-videoW/2.0f/glm::tan(glm::radians(SettingsVR::DEFAULT_FOV_FILLED_BY_SCENE/2.0f));
    videoZ*=1/(mSettingsVR.VR_SCENE_SCALE_PERCENTAGE/100.0f);
    updatePosition(videoZ,videoW,videoH);
    mOSDRenderer->onSurfaceSizeChanged(RENDER_TEX_W, RENDER_TEX_H);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jobject videoSurfaceTexture,jint videoSurfaceTextureId) {
    NDKThreadHelper::setProcessThreadPriority(env,FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    vrCompositorRenderer.initializeGL();
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times

    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,mTelemetryReceiver);
    videoTextureId=(GLuint)videoSurfaceTextureId;
    mSurfaceTextureUpdate.setSurfaceTexture(env,videoSurfaceTexture);
    //QCOM_tiled_rendering::init();
    //ANDROID_presentation_time::init();
    Extensions2::init();
    KHR_debug::enable();
    //
    osdRenderbuffer.initializeGL(RENDER_TEX_W,RENDER_TEX_H);
    //auto framebuffer_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
    //MLOGD<<"W "<<framebuffer_size.width<<"H "<<framebuffer_size.height;
}

void GLRStereoNormal::onSurfaceChanged(int width, int height) {
    placeGLElements();
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glClearColor(0,0,0,0.0F);
    cpuFrameTime.reset();
    WIDTH=width;
    HEIGHT=height;
}

// When we have VSYNC disabled ( which always means rendering into the front buffer directly) onDrawFrame is called as fast as possible.
// To not waste too much CPU & GPU on frames where the video did not change I limit the OpenGL FPS to max. 60fps here, but
// instead of sleeping I poll on the surfaceTexture in small intervalls to see if a new frame is available
// As soon as a new video frame is available, I render the OpenGL frame immediately
// This is not as efficient as using a condition variable but since the callback is invoked in java it might be hard to implement that
void GLRStereoNormal::waitUntilVideoFrameAvailable(JNIEnv* env,const std::chrono::steady_clock::time_point& maxWaitTimePoint) {
    if(const auto delay=mSurfaceTextureUpdate.waitUntilFrameAvailable(env,maxWaitTimePoint)){
        surfaceTextureDelay.add(*delay);
        MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
    }else{
        MLOGD<<"Timeout";
        ATrace_beginSection("Timeout");
        ATrace_endSection();
    }
}

void GLRStereoNormal::calculateFrameTimes() {
    // remove frames we are done with
    while(!mPendingFrames.empty()){
        const auto& submittedFrame=mPendingFrames.front();
        auto stats=Extensions2::getFrameTimestamps(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW),submittedFrame.frameId);
        if(stats){
            //Extensions2::logStats(submittedFrame.creationTime,*stats);
            MLOGD<<"To present "<<MyTimeHelper::R(std::chrono::nanoseconds(stats->DISPLAY_PRESENT_TIME_ANDROID-submittedFrame.creationTime.time_since_epoch().count()));
            mPendingFrames.pop();
        }else{
            break;
        }
    }
    auto thisFrame=Extensions2::getNextFrameId(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
    if(thisFrame){
        if(mPendingFrames.size()>4){
            mPendingFrames.pop();
        }
        mPendingFrames.push(Extensions2::SubmittedFrame{std::chrono::steady_clock::now(),*thisFrame});
    }
    //MLOGD<<"Frames in queue "<<mPendingFrames.size();
    //Extensions2::GetCompositorTimingANDROID(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW));
}

void GLRStereoNormal::onDrawFrame(JNIEnv* env) {
    ATrace_beginSection("GLRStereoNormal::onDrawFrame");
#ifdef CHANGE_SWAP_COLOR
    GLHelper::updateSetClearColor(swapColor);
#endif
    osdRenderbuffer.bind();
    glClearColor(1,0,0,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    mOSDRenderer->updateAndDrawElementsGL();
    osdRenderbuffer.unbind();
    glClearColor(0,0,0,0);
    if(checkAndResetVideoFormatChanged()){
        placeGLElements();
    }
    mFPSCalculator.tick();
    MLOGD<<"FPS"<<mFPSCalculator.getCurrentFPS();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    vrCompositorRenderer.updateLatestHeadSpaceFromStartSpaceRotation();
    if(mRenderingMode==SUBMIT_FRAMES){
        ATrace_beginSection("My updateVideoFrame");
        if(true){
            const std::chrono::steady_clock::time_point timeWhenWaitingExpires=lastRenderedFrame+std::chrono::milliseconds(33);
            waitUntilVideoFrameAvailable(env,timeWhenWaitingExpires);
        }else{
            if(const auto delay=mSurfaceTextureUpdate.updateAndCheck(env)){
                surfaceTextureDelay.add(*delay);
                //MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
            }
        }
        lastRenderedFrame=std::chrono::steady_clock::now();
        ATrace_endSection();
        //
        ATrace_beginSection("MglClear");
        //QCOM_tiled_rendering::glStartTilingQCOM(0,0,WIDTH,HEIGHT,0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        ATrace_endSection();
    }
    cpuFrameTime.start();
    drawEye(env,GVR_LEFT_EYE,false);
    drawEye(env,GVR_RIGHT_EYE, false);
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(std::chrono::seconds(5));
    //calculateFrameTimes();
    //ANDROID_presentation_time::eglPresentationTimeANDROID(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW),std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    ATrace_endSection();
    //QCOM_tiled_rendering::EndTilingQCOM();
}

void GLRStereoNormal::drawEye(JNIEnv* env,gvr::Eye eye,bool updateOSDBetweenEyes) {
    ATrace_beginSection((std::string("GLRStereoNormal::drawEye ")+(eye==0 ? "left" : "right")).c_str());
    if (mRenderingMode == SUBMIT_HALF_FRAMES) {
        QCOM_tiled_rendering::HalfFrameStartTilingQCOM(eye, WIDTH, HEIGHT);
        const std::chrono::steady_clock::time_point timeWhenWaitingExpires =
                lastRenderedFrame + std::chrono::milliseconds(5);
        waitUntilVideoFrameAvailable(env, timeWhenWaitingExpires);
        lastRenderedFrame = std::chrono::steady_clock::now();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    //
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_ADD);
    vrCompositorRenderer.drawLayers(eye);

    //glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_SUBTRACT);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

    if (mRenderingMode == SUBMIT_HALF_FRAMES) {
        QCOM_tiled_rendering::EndTilingQCOM();
        eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
    }
    ATrace_endSection();
}

void GLRStereoNormal::updatePosition(const float positionZ, const float width, const float height) {
    vrCompositorRenderer.removeLayers();
    const unsigned int TESSELATION_FACTOR=10;
    const auto headTrackingMode=mSettingsVR.GHT_MODE==0 ? VrCompositorRenderer::NONE:VrCompositorRenderer::FULL;

    //const auto vid0=TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,height},0.0f,1.0f);
    const auto vid1=VideoModesHelper::createMeshForMode(videoMode, positionZ, width, height);
    vrCompositorRenderer.addLayer(vid1,videoTextureId, true,headTrackingMode);

    const auto osd=TexturedGeometry::makeTesselatedVideoCanvas(TESSELATION_FACTOR,{0,0,positionZ},{width,width*1.0f/OSD_RATIO},0.0f,1.0f,false,false);
    vrCompositorRenderer.addLayer(osd, osdRenderbuffer.texture, false,headTrackingMode);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_play_1stereo_GLRStereoNormal_##method_name

inline jlong jptr(GLRStereoNormal *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRStereoNormal *native(jlong ptr) {
    return reinterpret_cast<GLRStereoNormal *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jlong telemetryReceiver, jlong native_gvr_api,jint videoMode) {
        return jptr(
            new GLRStereoNormal(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),videoMode));
}

JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject instance, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject androidContext,jobject videoSurfaceTexture,jint videoSurfaceTextureId) {
    native(glRendererStereo)->onSurfaceCreated(env,androidContext,videoSurfaceTexture,videoSurfaceTextureId);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->onSurfaceChanged(w, h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    native(glRendererStereo)->onDrawFrame(env);
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->SetVideoRatio((int)videoW,(int)videoH);
}

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrCompositorRenderer.updateHeadsetParams(deviceParams);
}

}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079