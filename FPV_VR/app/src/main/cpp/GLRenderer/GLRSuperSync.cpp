//
// Created by Constantin on 06.12.2017.
//

#include <cFiles/telemetry.h>
#include "GLRSuperSync.h"
#include "../SettingsN.h"
#include "../Helper/Extensions.h"
#include "../Helper/CPUPriorities.h"


#include "gvr.h"
#include "gvr_types.h"

#define TAG "GLRendererN"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


GLRSuperSync::GLRSuperSync(gvr_context *gvr_context) {
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    mTelemetryReceiver=std::make_shared<TelemetryReceiver>(S_OSD_ParseLTM,S_LTMPort,S_OSD_ParseFRSKY,S_FRSKYPort,S_OSD_ParseMAVLINK,
                                                           S_MAVLINKPort,S_OSD_ParseRSSI,S_RSSIPort);
    mHeadTrackerExtended=make_shared<HeadTrackerExtended>(S_InterpupilaryDistance);
}

GLRSuperSync::~GLRSuperSync() {
}

void GLRSuperSync::OnSurfaceCreated(JNIEnv *env, jobject obj, jint videoTexture,
                                    jobject assetManagerJAVA, bool qcomTiledRenderingAvailable) {
    //start the UDP telemetry receiving thread(s)
    mTelemetryReceiver->startReceiving();
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mGLRenderColor=std::make_shared<GLRenderColor>(S_DistortionCorrection);
    mGLRenderLine=std::make_shared<GLRenderLine>(S_DistortionCorrection);
    mGLRenderText=std::make_shared<GLRenderText>(S_DistortionCorrection);
    mGLRenderText->loadTextureImage(env, obj,assetManagerJAVA);
    mGLRenderTextureExternal=std::make_shared<GLRenderTextureExternal>(S_DistortionCorrection,(GLuint)videoTexture);
    mOSDRenderer=std::make_shared<OSDRenderer>(mTelemetryReceiver.get(), mGLRenderColor.get(),mGLRenderLine.get(), mGLRenderText.get());
    mVideoRenderer=std::make_shared<VideoRenderer>(mGLRenderColor.get(),mGLRenderTextureExternal.get(),S_DistortionCorrection);
    std::vector<std::string> v = {"startDR","updateTexImage1","clear","drawGL","stopDR","updateTexImage2"};
    mFrameCPUChronometer=std::make_shared<FrameCPUChronometer>(v);
    initOtherExtensions();
    videoFormatChanged=false;
    std::function<void(JNIEnv *env2, bool whichEye, int64_t offsetNS)> f = [=](JNIEnv *env2, bool whichEye, int64_t offsetNS) {
        this->renderNewEyeCallback(env2,whichEye,offsetNS);
    };
    mFBRManager=std::make_shared<FBRManager>(qcomTiledRenderingAvailable,f, nullptr);
}

void GLRSuperSync::OnSurfaceChanged(int width, int height) {
    WindowW=width;
    WindowH=height;
    ViewPortW=width/2;
    ViewPortH=height;
    mHeadTrackerExtended->calculateMatrices(((float) ViewPortW)/((float)ViewPortH));
    placeGLElements();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f,0,0,0.0f);
    glEnable(GL_SCISSOR_TEST);
}

void GLRSuperSync::enterSuperSyncLoop(JNIEnv * env, jobject obj,jobject surfaceTexture,int exclusiveVRCore) {
    mVideoRenderer->initUpdateTexImageJAVA(env,obj,surfaceTexture);
    setAffinity(exclusiveVRCore);
    setCPUPriority(CPU_PRIORITY_GLRENDERER_STEREO_FB,"GLRenderer14_StereoSuperSYNC");
    mTelemetryReceiver->get_other_osd_data()->opengl_fps=-1.0f;
    //This will block until mFBRManager->exitDirectRenderingLoop() is called
    LOGV("entering superSync loop. GLThread will be blocked");
    mFBRManager->enterDirectRenderingLoop(env);
    LOGV("exited superSync loop. GLThread unblocked");
    mVideoRenderer->deleteUpdateTexImageJAVA(env,obj);
}

void GLRSuperSync::renderNewEyeCallback(JNIEnv *env, bool whichEye, int64_t offsetNS) {
    //changeSwapColor=true;
    if(changeSwapColor){
        color++;
        if(color>4){
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            color=0;
        }else{
            glClearColor(1.0f,1.0f,0.0f,0.0f);
        }
    }
    if(videoFormatChanged){
        placeGLElements();
        videoFormatChanged=false;
    }
    mFrameCPUChronometer->start(whichEye);
    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        //when using QCOM tiled rendering, we call 'startDirectRendering()' before 'updateTexImage()'
        mFBRManager->startDirectRendering(whichEye,ViewPortW,ViewPortH);
    }
    mFrameCPUChronometer->setTimestamp(whichEye,0);
    //this probably implies a glFlush(). The problem is that a glFlush() also implies a glEndTilingQcom
    //So we should not call glClear() (or any glCalls that may affect the fb values) before updateTexImageJAVA().
    //or else the GPU might have to copy tiles 2 times
    mVideoRenderer->updateTexImageJAVA(env);
    mFrameCPUChronometer->setTimestamp(whichEye,1);
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
        //TODO: Determine if a missing clear results in image artifacts or has any other disadvantages
        //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    mFrameCPUChronometer->setTimestamp(whichEye,2);
    drawEye(whichEye);
    mFrameCPUChronometer->setTimestamp(whichEye,3);
    mFBRManager->stopDirectRendering(whichEye);
    mFrameCPUChronometer->setTimestamp(whichEye,4);
    //update the video texture again. If there is no new surfaceTexture Data available, this call returns almost immediately anyway.
    //and only if the decoder produces a stream with >120fps, we "waste" GPU cycles on a surfaceTexture update that won't make it to the screen
    //I don't expect the video stream to have >120fps for a real-time live video feed
    //I am also not quite sure, if the 'updateTexImage()' at the beginning of the frame actually makes it to the eye that is drawn after it
    if(mFBRManager->directRenderingMode==FBRManager::QCOM_TILED_RENDERING){
        mVideoRenderer->updateTexImageJAVA(env);
    }
    mFrameCPUChronometer->setTimestamp(whichEye,5);
    mFrameCPUChronometer->stop(whichEye);
    mFrameCPUChronometer->print();
}
void GLRSuperSync::drawEye(bool whichEye) {
    if(mHeadTrackerExtended->mGroundTrackingMode!=HeadTrackerExtended::MODE_NONE){
        mHeadTrackerExtended->calculateNewHeadPose(gvr_api_.get(),16);
    }
    WorldMatrices* worldMatrices=mHeadTrackerExtended->getWorldMatrices();
    if(mHeadTrackerExtended->mGroundTrackingMode==HeadTrackerExtended::MODE_1PP){
        if(whichEye){
            mVideoRenderer->drawGL(worldMatrices->leftEyeViewTracked,worldMatrices->projection);
            mOSDRenderer->updateAndDrawElements(worldMatrices->leftEyeViewTracked,worldMatrices->leftEyeViewTracked,worldMatrices->projection, false);
        }else {
            mVideoRenderer->drawGL(worldMatrices->rightEyeViewTracked,worldMatrices->projection);
            mOSDRenderer->updateAndDrawElements(worldMatrices->rightEyeViewTracked,worldMatrices->rightEyeViewTracked,worldMatrices->projection,false);
        }
    }else{
        bool t=mHeadTrackerExtended->mGroundTrackingMode==HeadTrackerExtended::MODE_3PP_ARTIFICIALHORIZON_ONLY;
        if(whichEye){
            mVideoRenderer->drawGL(worldMatrices->leftEyeView,worldMatrices->projection);
            mOSDRenderer->updateAndDrawElements(worldMatrices->leftEyeView,worldMatrices->leftEyeViewTracked,worldMatrices->projection,t);
        }else{
            mVideoRenderer->drawGL(worldMatrices->rightEyeView,worldMatrices->projection);
            mOSDRenderer->updateAndDrawElements(worldMatrices->rightEyeView,worldMatrices->rightEyeViewTracked,worldMatrices->projection,t);
        }
    }
}

void GLRSuperSync::placeGLElements() {
    int strokeW=2;
    float videoW=5;
    float videoH=videoW*1.0f/videoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    float videoZ=-MAX_Z_DISTANCE*((100-S_SceneScale)/100.0f);
    mOSDRenderer->placeGLElementsStereo(videoX,videoY,videoZ,videoW,videoH,strokeW);
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRSuperSync::exitSuperSyncLoop() {
    mFBRManager->exitDirectRenderingLoop();
}

void GLRSuperSync::OnGLSurfaceDestroyed() {
    if(mTelemetryReceiver.get()){
        mTelemetryReceiver->stopReceiving();
    }
    if(mOSDRenderer.get()){
        mOSDRenderer->stop();
    }
}

void GLRSuperSync::OnVideoRatioChanged(int videoW, int videoH) {
    videoFormat=((float)videoW)/(float)videoH;
    videoFormatChanged=true;
}

void GLRSuperSync::doFrame(int64_t lastVSYNC) {
    if(mFBRManager){
        mFBRManager->interpretVSYNC(lastVSYNC);
    }
}

void GLRSuperSync::setVideoDecoderFPS(float fps) {
    if(mTelemetryReceiver.get()){
        mTelemetryReceiver->get_other_osd_data()->decoder_fps=fps;
    }
}
void GLRSuperSync::setHomeLocation(double latitude, double longitude,double attitude) {
    mTelemetryReceiver->setHome(latitude,longitude,attitude);
}
//----------------------------------------------------JAVA bindings---------------------------------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_GLRenderer14_1StereoSuperSYNC_##method_name

inline jlong jptr(GLRSuperSync *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}

inline GLRSuperSync *native(jlong ptr) {
    return reinterpret_cast<GLRSuperSync *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstructRendererStereo)
(JNIEnv *env, jclass clazz, jlong native_gvr_api) {
    return jptr(
            new GLRSuperSync(reinterpret_cast<gvr_context *>(native_gvr_api)));
}

JNI_METHOD(void, nativeDestroyRendererStereo)
(JNIEnv *env, jclass clazz, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoTexture,jobject assetManagerJAVA,jboolean qcomTiledRenderingAvailable) {
    LOGV("nativeOnSurfaceCreated()");
    native(glRendererStereo)->OnSurfaceCreated(env,obj,videoTexture,assetManagerJAVA,(bool)qcomTiledRenderingAvailable);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    LOGV("nativeOnSurfaceChanged() %dx%d",(int)w,(int)h);
    native(glRendererStereo)->OnSurfaceChanged(w,h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jobject surfaceTexture,jint exclusiveVRCore) {
    LOGV("nativeOnDrawFrame()");
    native(glRendererStereo)->enterSuperSyncLoop(env,obj, surfaceTexture,(int)exclusiveVRCore);
}

JNI_METHOD(void, nativeExitSuperSyncLoop)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    LOGV("nativeExitSuperSyncLoop()");
    native(glRendererStereo)->exitSuperSyncLoop();
}

JNI_METHOD(void, nativeOnGLSurfaceDestroyed)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    LOGV("nativeOnGLSurfaceDestroyed()");
    native(glRendererStereo)->OnGLSurfaceDestroyed();
}

JNI_METHOD(void, nativeDoFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jlong lastVSYNC) {
    native(glRendererStereo)->doFrame((int64_t)lastVSYNC);
}

JNI_METHOD(void, nativeSetVideoDecoderFPS)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jfloat decFPS) {
    native(glRendererStereo)->setVideoDecoderFPS((float)decFPS);
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->OnVideoRatioChanged((int)videoW,(int)videoH);
}
JNI_METHOD(void, nativeSetHomeLocation)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jdouble latitude,jdouble longitude,jdouble attitude) {
    native(glRendererStereo)->setHomeLocation((double)latitude,(double)longitude,(double)attitude);
    LOGV("setHomeLocation()");
}
}