
#include "GLRStereoNormal.h"
#include "jni.h"
#include "../Scene/Video/VideoRenderer.h"
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

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,const int videoMode):
mSurfaceTextureUpdate(env),
videoMode(static_cast<VideoRenderer::VIDEO_RENDERING_MODE>(videoMode)),mSettingsVR(env,androidContext),
distortionManager(mSettingsVR.VR_DISTORTION_CORRECTION_MODE==0 ? VDDCManager::NONE : VDDCManager::RADIAL_CARDBOARD),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
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
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->updatePosition(videoZ,videoW,videoH,lastVideoWidthPx,lastVideoHeightPx);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jobject videoSurfaceTexture,jint videoSurfaceTextureId) {
    NDKThreadHelper::setProcessThreadPriority(env,FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(&distortionManager);
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mVideoRenderer=std::make_unique<VideoRenderer>(videoMode,(GLuint)videoSurfaceTextureId,&distortionManager);
    const auto color=TrueColor2::BLACK;
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(vrHeadsetParams,color,mOcclusionMesh);
    mSurfaceTextureUpdate.setSurfaceTexture(env,videoSurfaceTexture);
}

void GLRStereoNormal::onSurfaceChanged(int width, int height) {
    placeGLElements();
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glClearColor(0,0,0,0.0F);
    cpuFrameTime.reset();

}

void GLRStereoNormal::onDrawFrame(JNIEnv* env) {
#ifdef CHANGE_SWAP_COLOR
    swapColor++;
        if(swapColor>1){
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            swapColor=0;
        }else{
            glClearColor(1.0f,1.0f,0.0f,0.0f);
        }
#endif
    if(checkAndResetVideoFormatChanged()){
        placeGLElements();
    }
    if(true){
        // When we have VSYNC disabled ( which always means rendering into the front buffer directly) onDrawFrame is called as fast as possible.
        // To not waste too much CPU & GPU on frames where the video did not change I limit the OpenGL FPS to max. 60fps here, but
        // instead of sleeping I poll on the surfaceTexture in small intervalls to see if a new frame is available
        // As soon as a new video frame is available, I render the OpenGL frame immediately
        while(true){
            if(const auto delay=mSurfaceTextureUpdate.updateAndCheck(env)){
                surfaceTextureDelay.add(*delay);
                MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
                break;
            }
            if((std::chrono::steady_clock::now()-lastRenderedFrame)>std::chrono::milliseconds(16)){
                break;
            }
            TestSleep::sleep(std::chrono::milliseconds(1));
            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }else{
        if(const auto delay=mSurfaceTextureUpdate.updateAndCheck(env)){
            surfaceTextureDelay.add(*delay);
            MLOGD<<"avg Latency until opengl is "<<surfaceTextureDelay.getAvg_ms();
        }
    }
    lastRenderedFrame=std::chrono::steady_clock::now();
    mFPSCalculator.tick();
    vrHeadsetParams.updateLatestHeadSpaceFromStartSpaceRotation();
    //start rendering the frame
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    drawEye(GVR_LEFT_EYE,false);
    drawEye(GVR_RIGHT_EYE, false);
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(std::chrono::seconds(5));
}

void GLRStereoNormal::drawEye(gvr::Eye eye,bool updateOSDBetweenEyes){
    distortionManager.setEye(eye==GVR_LEFT_EYE);
    vrHeadsetParams.setOpenGLViewport(eye);
    //Now draw
    const auto rotation=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation();
    glm::mat4 viewVideo;
    glm::mat4 viewOSD;
    if(mVideoRenderer->is360Video()){
        //When rendering 360° video,always fully track head rotation for video, optionally lock OSD
        viewVideo= vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotation;
        viewOSD= mSettingsVR.GHT_OSD_FIXED_TO_HEAD ? vrHeadsetParams.GetEyeFromHeadMatrix(eye):
                vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotation;
    }else{
        //Else, track video only if head tracking is enabled, lock OSD if requested
        const auto rotationWithAxesDisabled=removeRotationAroundSpecificAxes(rotation,mSettingsVR.GHT_X,mSettingsVR.GHT_Y,mSettingsVR.GHT_Z);
        viewVideo=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotationWithAxesDisabled;
        viewOSD= mSettingsVR.GHT_OSD_FIXED_TO_HEAD ? vrHeadsetParams.GetEyeFromHeadMatrix(eye)
                                                   : vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotationWithAxesDisabled;
    }
    const glm::mat4 projection=vrHeadsetParams.GetProjectionMatrix(eye);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_ADD);
    mVideoRenderer->drawVideoCanvas(viewVideo,projection,eye==GVR_LEFT_EYE);
    if(eye==GVR_LEFT_EYE || updateOSDBetweenEyes){
        mOSDRenderer->updateAndDrawElementsGL(viewOSD,projection);
    }else{
        mOSDRenderer->drawElementsGL(viewOSD,projection);
    }
    //glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_SUBTRACT);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
    //mVideoRenderer->drawVideoCanvas(viewVideo,projection,eye==GVR_LEFT_EYE);

    //Render the mesh that occludes everything except the part actually visible inside the headset
    if(mSettingsVR.VR_DISTORTION_CORRECTION_MODE!=0){
        int idx=eye==GVR_LEFT_EYE ? 0 : 1;
        mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[idx]);
    }
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
    native(instancePointer)->vrHeadsetParams.updateHeadsetParams(deviceParams);
    native(instancePointer)->vrHeadsetParams.updateDistortionManager(native(instancePointer)->distortionManager);
}

}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079