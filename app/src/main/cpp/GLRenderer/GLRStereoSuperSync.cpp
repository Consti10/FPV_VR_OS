//
// Created by Constantin on 06.12.2017.
//

#include "GLRStereoSuperSync.h"
#include "Extensions.hpp"
#include "CPUPriorities.hpp"


#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define TAG "GLRendererN"
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#define CHANGE_SWAP_COLOR

GLRStereoSuperSync::GLRStereoSuperSync(JNIEnv* env,jobject androidContext,jfloatArray undistortionData,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,bool qcomTiledRenderingAvailable,bool reusableSyncAvailable,bool is360):
GLRStereoNormal(env,androidContext,telemetryReceiver,gvr_context,is360),
        mFrameTimeAcc(std::vector<std::string>{"startDR","updateTexImage1","clear","drawVideoCanvas","stopDR","updateTexImage2"}){
    std::function<void(JNIEnv *env2, bool whichEye, int64_t offsetNS)> f = [this](JNIEnv *env2, bool whichEye, int64_t offsetNS) {
        this->renderNewEyeCallback(env2,whichEye,offsetNS);
    };
    mFBRManager=std::make_unique<FBRManager>(qcomTiledRenderingAvailable,reusableSyncAvailable,true,f, nullptr);
}

void GLRStereoSuperSync::onSurfaceCreatedX(JNIEnv *env, jobject androidContext, jint videoTexture) {
   GLRStereoNormal::onSurfaceCreated(env,androidContext,videoTexture);
    mFrameTimeAcc.reset();
    initOtherExtensions();
}

void GLRStereoSuperSync::onSurfaceChangedX(int width, int height) {
    GLRStereoNormal::onSurfaceChanged(width,height);
    glEnable(GL_SCISSOR_TEST);
}

void GLRStereoSuperSync::enterSuperSyncLoop(JNIEnv * env, jobject obj,jobject surfaceTexture,int exclusiveVRCore) {
    mVideoRenderer->initUpdateTexImageJAVA(env,obj,surfaceTexture);
    setAffinity(exclusiveVRCore);
    setCPUPriority(CPU_PRIORITY_GLRENDERER_STEREO_FB,"GLRStereoSuperSync");
    mTelemetryReceiver.setOpenGLFPS(-1);
    //This will block until mFBRManager->requestExitSuperSyncLoop() is called
    LOGD("entering superSync loop. GLThread will be blocked");
    mFBRManager->enterDirectRenderingLoop(env);
    LOGD("exited superSync loop. GLThread unblocked");
    mVideoRenderer->deleteUpdateTexImageJAVA(env,obj);
}

void GLRStereoSuperSync::exitSuperSyncLoop() {
    mFBRManager->requestExitSuperSyncLoop();
}

void GLRStereoSuperSync::renderNewEyeCallback(JNIEnv *env, bool whichEye, int64_t offsetNS) {
#ifdef CHANGE_SWAP_COLOR
    swapColor++;
        if(swapColor>2){
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            swapColor=0;
        }else{
            glClearColor(1.0f,1.0f,0.0f,0.0f);
        }
#endif
    if(videoFormatChanged){
        placeGLElements();
        videoFormatChanged=false;
    }
    VREyeDurations vrEyeTimeStamps{whichEye};

    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        //when using QCOM tiled rendering, we call 'startDirectRendering()' before 'updateTexImage()'
        mFBRManager->startDirectRendering(whichEye,vrHeadsetParams.screenWidthP/2,vrHeadsetParams.screenHeightP);
    }
    vrEyeTimeStamps.setTimestamp("startDR");
    //this probably implies a glFlush(). The problem is that a glFlush() also implies a glEndTilingQcom
    //So we should not call glClear() (or any glCalls that may affect the fb values) before updateTexImageJAVA().
    //or else the GPU might have to copy tiles 2 times
    mVideoRenderer->updateTexImageJAVA(env);
    vrEyeTimeStamps.setTimestamp("updateTexImage1");
    if(mFBRManager->directRenderingMode!=FBRManager::QCOM_TILED_RENDERING){
        //when not using QCOM tiled rendering, we call 'startDirectRendering()' after 'updateTexImage()'
        mFBRManager->startDirectRendering(whichEye,vrHeadsetParams.screenWidthP/2,vrHeadsetParams.screenHeightP);
    }
    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        //so we have to call glClear() before any OpenGL calls that affect framebuffer contents (e.g. draw())
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }else{
        //we only need a clear when we are using QCOM tiled rendering. MALI GPUs seem to not like clears.
        //see PowerVR.Performance*Recommendations.pdf,6.1: "performing a glClear on half the frame is a "particularClear".
        //according to the article, a particular clear means the GPU has to read in the whole frame first, and then "overdraw" the area that has to be cleared
        //when doing front buffer rendering btw. rendering half-screen images we can only clear half the screen -> therefore I do not call glClear.
        //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    vrEyeTimeStamps.setTimestamp("clear");
    vrHeadsetParams.updateLatestHeadSpaceFromStartSpaceRotation();
    GLRStereoNormal::drawEye(whichEye ? GVR_LEFT_EYE : GVR_RIGHT_EYE,true);
    vrEyeTimeStamps.setTimestamp("drawVideoCanvas");
    mFBRManager->stopDirectRendering(whichEye);
    vrEyeTimeStamps.setTimestamp("stopDR");
    //update the video texture again. If there is no new surfaceTexture Data available, this call returns almost immediately anyway.
    //and only if the decoder produces a stream with >120fps, we "waste" GPU cycles on a surfaceTexture update that won't make it to the screen
    //I don't expect the video stream to have >120fps for a real-time live video feed
    //I am also not quite sure, if the 'updateTexImage()' at the beginning of the frame actually makes it to the eye that is drawn after it
    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        mVideoRenderer->updateTexImageJAVA(env);
    }
    vrEyeTimeStamps.setTimestamp("updateTexImage2");
    //vrEyeTimeStamps.print();
    mFrameTimeAcc.add(vrEyeTimeStamps);
    mFrameTimeAcc.printEveryXSeconds(5);
}


void GLRStereoSuperSync::setLastVSYNC(int64_t lastVSYNC) {
    mFBRManager->setLastVSYNC(lastVSYNC);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_PlayStereo_GLRStereoSuperSync_##method_name

inline jlong jptr(GLRStereoSuperSync *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRStereoSuperSync *native(jlong ptr) {
    return reinterpret_cast<GLRStereoSuperSync *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jfloatArray undistortionData,jlong telemetryReceiver, jlong native_gvr_api,jboolean qcomTiledRenderingAvailable,jboolean reusableSyncAvailable,jboolean is360) {
    return jptr(
            new GLRStereoSuperSync(env,androidContext,undistortionData,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),(bool)qcomTiledRenderingAvailable,(bool)reusableSyncAvailable,(bool)is360));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoTexture,jobject androidContext) {
    native(glRendererStereo)->onSurfaceCreatedX(env,androidContext,videoTexture);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->onSurfaceChangedX(w, h);
}
JNI_METHOD(void, nativeEnterSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject surfaceTexture,jint exclusiveVRCore) {
    LOGD("nativeEnterSuperSyncLoop()");
    native(glRendererStereo)->enterSuperSyncLoop(env,obj, surfaceTexture,(int)exclusiveVRCore);
}
JNI_METHOD(void, nativeExitSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    LOGD("nativeExitSuperSyncLoop()");
    native(glRendererStereo)->exitSuperSyncLoop();
}
JNI_METHOD(void, nativeDoFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jlong lastVSYNC) {
    native(glRendererStereo)->setLastVSYNC((int64_t) lastVSYNC);
}
JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->SetVideoRatio((int)videoW,(int)videoH);
}

JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instance,
 jfloat screen_width_meters,
 jfloat screen_height_meters,
 jfloat screen_to_lens_distance,
 jfloat inter_lens_distance,
 jint vertical_alignment,
 jfloat tray_to_lens_distance,
 jfloatArray device_fov_left,
 jfloatArray radial_distortion_params,
 jint screenWidthP,jint screenHeightP
) {
    std::array<float,4> device_fov_left1{};
    std::vector<float> radial_distortion_params1(2);

    jfloat *arrayP=env->GetFloatArrayElements(device_fov_left, nullptr);
    std::memcpy(device_fov_left1.data(),&arrayP[0],4*sizeof(float));
    env->ReleaseFloatArrayElements(device_fov_left,arrayP,0);
    arrayP=env->GetFloatArrayElements(radial_distortion_params, nullptr);
    std::memcpy(radial_distortion_params1.data(),&arrayP[0],2*sizeof(float));
    env->ReleaseFloatArrayElements(radial_distortion_params,arrayP,0);

    const MDeviceParams deviceParams{screen_width_meters,screen_height_meters,screen_to_lens_distance,inter_lens_distance,vertical_alignment,tray_to_lens_distance,
                                     device_fov_left1,radial_distortion_params1};
    native(instance)->vrHeadsetParams.updateHeadsetParams(deviceParams,screenWidthP,screenHeightP);
    native(instance)->vrHeadsetParams.updateDistortionManager(native(instance)->distortionManager);
}

}