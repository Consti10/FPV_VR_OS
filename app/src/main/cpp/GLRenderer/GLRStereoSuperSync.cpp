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
is360(is360),
        mTelemetryReceiver(telemetryReceiver),
        mFrameTimeAcc(std::vector<std::string>{"startDR","updateTexImage1","clear","drawVideoCanvas","stopDR","updateTexImage2"}),
        mSettingsVR(env,androidContext,undistortionData,gvr_context),
        mMatricesM(mSettingsVR){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    std::function<void(JNIEnv *env2, bool whichEye, int64_t offsetNS)> f = [this](JNIEnv *env2, bool whichEye, int64_t offsetNS) {
        this->renderNewEyeCallback(env2,whichEye,offsetNS);
    };
    mFBRManager=std::make_unique<FBRManager>(qcomTiledRenderingAvailable,reusableSyncAvailable,true,f, nullptr);
}

void GLRStereoSuperSync::placeGLElements() {
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    float viewPortRatio=(float)ViewPortW/ViewPortH;
    float videoZ=-videoW/2.0f/viewPortRatio/glm::tan(glm::radians(MAX_FOV_USABLE_FOR_VDDC/2.0f));
    videoZ*=1.1;
    videoZ*=(100-mSettingsVR.VR_SceneScale)/100.0f*2;
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRStereoSuperSync::onSurfaceCreated(JNIEnv *env, jobject androidContext, jint videoTexture) {
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times

    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(mSettingsVR.getDistortionManager());
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mGLRenderTextureExternal=std::make_unique<GLProgramTexture>(true,mSettingsVR.getDistortionManager());
    mVideoRenderer=std::make_unique<VideoRenderer>(is360 ? VideoRenderer::VIDEO_RENDERING_MODE::RM_360_EQUIRECTANGULAR :VideoRenderer::VIDEO_RENDERING_MODE::RM_NORMAL,
            (GLuint)videoTexture,mBasicGLPrograms->vc,mGLRenderTextureExternal.get());
    mFrameTimeAcc.reset();
    initOtherExtensions();
    videoFormatChanged=false;
}

void GLRStereoSuperSync::onSurfaceChanged(int width, int height) {
    ViewPortW=width/2;
    ViewPortH=height;
    placeGLElements();
    mMatricesM.calculateProjectionAndDefaultView(MAX_FOV_USABLE_FOR_VDDC,
                                                 ((float) ViewPortW) / ((float) ViewPortH));
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f,0,0,0.0f);
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
        mFBRManager->startDirectRendering(whichEye,ViewPortW,ViewPortH);
    }
    vrEyeTimeStamps.setTimestamp("startDR");
    //this probably implies a glFlush(). The problem is that a glFlush() also implies a glEndTilingQcom
    //So we should not call glClear() (or any glCalls that may affect the fb values) before updateTexImageJAVA().
    //or else the GPU might have to copy tiles 2 times
    mVideoRenderer->updateTexImageJAVA(env);
    vrEyeTimeStamps.setTimestamp("updateTexImage1");
    if(mFBRManager->directRenderingMode!=FBRManager::QCOM_TILED_RENDERING){
        //when not using QCOM tiled rendering, we call 'startDirectRendering()' after 'updateTexImage()'
        mFBRManager->startDirectRendering(whichEye,ViewPortW,ViewPortH);
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
    drawEye(whichEye);
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

void GLRStereoSuperSync::drawEye(bool whichEye) {
    mMatricesM.calculateNewHeadPoseIfNeeded(gvr_api_.get(), 16);
    glm::mat4x4 leftEye,rightEye,projection;
    if(mMatricesM.settingsVR.GHT_MODE==MatricesManager::MODE_1PP){
        Matrices& worldMatrices=mMatricesM.getWorldMatrices();
        leftEye=worldMatrices.leftEyeViewTracked;
        rightEye=worldMatrices.rightEyeViewTracked;
        projection=worldMatrices.projection;
    }else{
        Matrices& worldMatrices=mMatricesM.getWorldMatrices();
        leftEye=worldMatrices.leftEyeView;
        rightEye=worldMatrices.rightEyeView;
        projection=worldMatrices.projection;
    }
    if(mSettingsVR.getDistortionManager()){
        mSettingsVR.getDistortionManager()->leftEye=whichEye;
    }
    if(whichEye){
        glViewport(0,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawVideoCanvas(leftEye,projection,true);
        mOSDRenderer->updateAndDrawElementsGL(leftEye,projection);
    }else{
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawVideoCanvas(rightEye,projection,false);
        mOSDRenderer->updateAndDrawElementsGL(rightEye,projection);
    }
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
    native(glRendererStereo)->OnSurfaceCreated(env,androidContext,videoTexture);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->OnSurfaceChanged(w, h);
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
}