
#include "GLRendererStereo.h"
#include "jni.h"
#include "OSDRenderer.h"
#include "VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderTextureExternal.h>
#include <TelemetryReceiver.h>
#include <Chronometer.h>
#include <cFiles/telemetry.h>
#include "../SettingsN.h"
#include "../Helper/CPUPriorities.h"

#include "gvr.h"
#include "gvr_types.h"

#define PRINT_LOGS
#define TAG "GLRendererStereo"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


GLRendererStereo::GLRendererStereo(gvr_context *gvr_context) {
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    mTelemetryReceiver=std::make_shared<TelemetryReceiver>(S_OSD_ParseLTM,S_LTMPort,S_OSD_ParseFRSKY,S_FRSKYPort,S_OSD_ParseMAVLINK,
                                                           S_MAVLINKPort,S_OSD_ParseRSSI,S_RSSIPort);
    mHeadTrackerExtended=make_shared<HeadTrackerExtended>(S_InterpupilaryDistance);
}

GLRendererStereo::~GLRendererStereo() {
}

void GLRendererStereo::placeGLElements(){
    //LOGV("Place GL Elements");
    int strokeW=2;
    float videoW=5;
    float videoH=videoW*1.0f/videoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    float videoZ=-HeadTrackerExtended::MAX_Z_DISTANCE*((100-S_SceneScale)/100.0f);
    mOSDRenderer->placeGLElementsStereo(videoX,videoY,videoZ,videoW,videoH,strokeW);
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRendererStereo::drawScene() {
    if(mHeadTrackerExtended->mGroundTrackingMode!=HeadTrackerExtended::MODE_NONE){
        mHeadTrackerExtended->calculateNewHeadPose(gvr_api_.get(),16);
    }
    WorldMatrices* worldMatrices=mHeadTrackerExtended->getWorldMatrices();
    //update and draw the eyes
    if(mHeadTrackerExtended->mGroundTrackingMode==HeadTrackerExtended::MODE_1PP){
        glViewport(0,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawGL(worldMatrices->leftEyeViewTracked,worldMatrices->projection);
        mOSDRenderer->updateAndDrawElements(worldMatrices->leftEyeViewTracked,worldMatrices->leftEyeViewTracked,worldMatrices->projection, false);
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawGL(worldMatrices->rightEyeViewTracked,worldMatrices->projection);
        mOSDRenderer->drawElementsGL(worldMatrices->rightEyeViewTracked,worldMatrices->rightEyeViewTracked,worldMatrices->projection,false);
    }else{
        bool t=mHeadTrackerExtended->mGroundTrackingMode==HeadTrackerExtended::MODE_3PP_ARTIFICIALHORIZON_ONLY;
        glViewport(0,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawGL(worldMatrices->leftEyeView,worldMatrices->projection);
        mOSDRenderer->updateAndDrawElements(worldMatrices->leftEyeView,worldMatrices->leftEyeViewTracked,worldMatrices->projection,t);
        glViewport(ViewPortW,0,ViewPortW,ViewPortH);
        mVideoRenderer->drawGL(worldMatrices->rightEyeView,worldMatrices->projection);
        mOSDRenderer->drawElementsGL(worldMatrices->rightEyeView,worldMatrices->rightEyeViewTracked,worldMatrices->projection,t);
    }
}

void GLRendererStereo::calculateMetrics(){
    int64_t ts=getTimeMS();
    mFPSCalculator->tick();
    mTelemetryReceiver->get_other_osd_data()->opengl_fps=mFPSCalculator->getCurrentFPS();
    //calculate and print CPU Frame time every 5 seconds
    if(ts-lastLog>5*1000){
        lastLog=ts;
#ifdef PRINT_LOGS
        CPUFrameTime->printAvg();
#endif
        CPUFrameTime->reset();
    }
}

void GLRendererStereo::OnSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture, jobject assetManagerJAVA) {
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
    lastLog=getTimeMS();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f,0,0,0.0f);
    gvr_api_->ResumeTracking();
    //gvr_api_->RecenterTracking();
    //gvr_api_->InitializeGl();
}

void GLRendererStereo::OnSurfaceChanged(int width, int height) {
    WindowW=width;
    WindowH=height;
    ViewPortW=width/2;
    ViewPortH=height;
    mHeadTrackerExtended->calculateMatrices(((float) ViewPortW)/((float)ViewPortH));
    placeGLElements();
    setCPUPriority(CPU_PRIORITY_GLRENDERER_MONO,TAG);
}

void GLRendererStereo::OnDrawFrame() {
    //changeSwapColor=true;
    if(changeSwapColor){
        color++;
        if(color>1){
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
    calculateMetrics();
    CPUFrameTime->start();
    //glScissor(0,0,WindowW,WindowH);
    //glViewport(0,0,WindowW,WindowH);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    drawScene();
    CPUFrameTime->stop();
}

void GLRendererStereo::OnGLSurfaceDestroyed() {
    if(mTelemetryReceiver.get()){
        mTelemetryReceiver->stopReceiving();
    }
    if(mOSDRenderer.get()){
        mOSDRenderer->stop();
    }
    gvr_api_->PauseTracking();
}

void GLRendererStereo::OnVideoRatioChanged(int videoW, int videoH) {
    //LOGV("Native video W:%d H:%d",videoW,videoH);
    videoFormat=((float)videoW)/(float)videoH;
    videoFormatChanged=true;
}

void GLRendererStereo::setVideoDecoderFPS(float fps) {
    if(mTelemetryReceiver.get()){
       mTelemetryReceiver->get_other_osd_data()->decoder_fps=fps;
    }
}

/*void GLRendererStereo::OnPause() {
    if(mTelemetryReceiver.get()){
        mTelemetryReceiver->stopReceiving();
    }
    if(mOSDRenderer.get()){
        mOSDRenderer->stop();
    }
}*/

void GLRendererStereo::setHomeLocation(double latitude, double longitude,double attitude) {
    mTelemetryReceiver->setHome(latitude,longitude,attitude);
}
//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_GLRenderer14_1Stereo_##method_name

inline jlong jptr(GLRendererStereo *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}

inline GLRendererStereo *native(jlong ptr) {
    return reinterpret_cast<GLRendererStereo *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstructRendererStereo)
(JNIEnv *env, jclass clazz, jlong native_gvr_api) {
    LOGV("create()");
    return jptr(
            new GLRendererStereo(reinterpret_cast<gvr_context *>(native_gvr_api)));
}

JNI_METHOD(void, nativeDestroyRendererStereo)
(JNIEnv *env, jclass clazz, jlong glRendererStereo) {
    LOGV("delete()");
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoTexture,jobject assetManagerJAVA) {
    LOGV("OnSurfaceCreated()");
    native(glRendererStereo)->OnSurfaceCreated(env,obj,videoTexture,assetManagerJAVA);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    LOGV("OnSurfaceChanged %dx%d",(int)w,(int)h);
    native(glRendererStereo)->OnSurfaceChanged(w,h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    //LOGV("OnDrawFrame()");
    native(glRendererStereo)->OnDrawFrame();
}

JNI_METHOD(void, nativeSetVideoDecoderFPS)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jfloat decFPS) {
    native(glRendererStereo)->setVideoDecoderFPS((float)decFPS);
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->OnVideoRatioChanged((int)videoW,(int)videoH);
}

JNI_METHOD(void, nativeOnGLSurfaceDestroyed)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    LOGV("nativeOnGLSurfaceDestroyed()");
    native(glRendererStereo)->OnGLSurfaceDestroyed();
}
JNI_METHOD(void, nativeSetHomeLocation)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jdouble latitude,jdouble longitude,jdouble attitude) {
    native(glRendererStereo)->setHomeLocation((double)latitude,(double)longitude,(double)attitude);
    LOGV("setHomeLocation()");
}
}
