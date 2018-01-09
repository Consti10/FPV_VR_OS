

#include <android/log.h>
#include <cFiles/telemetry.h>
#include <GLRenderLine.h>
#include "GLRendererMono.h"
#include "../SettingsN.h"
#include "../Helper/CPUPriorities.h"


#define TAG "GLRendererMono"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define PRINT_LOGS


GLRendererMono::GLRendererMono(gvr_context *gvr_context) {
    //LOGV("construct!");
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    mTelemetryReceiver=std::make_shared<TelemetryReceiver>(S_OSD_ParseLTM,S_LTMPort,S_OSD_ParseFRSKY,S_FRSKYPort,S_OSD_ParseMAVLINK,
                                                           S_MAVLINKPort,S_OSD_ParseRSSI,S_RSSIPort);
}

GLRendererMono::~GLRendererMono() {
    //LOGV("delete");
}

void GLRendererMono::OnSurfaceCreated(JNIEnv * env,jobject obj,jobject assetManagerJAVA) {
    //start the UDP telemetry receiving thread(s)
    mTelemetryReceiver->startReceiving();
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mGLRenderColor=std::make_shared<GLRenderColor>(false);
    mGLRenderLine=std::make_shared<GLRenderLine>(false);
    mGLRenderText=std::make_shared<GLRenderText>(false);
    mGLRenderText->loadTextureImage(env, obj,assetManagerJAVA);
    mOSDRenderer=std::make_shared<OSDRenderer>(mTelemetryReceiver.get(), mGLRenderColor.get(),mGLRenderLine.get(), mGLRenderText.get());
    lastLog=getTimeMS();
    CPUFrameTime->reset();
}

void GLRendererMono::OnSurfaceChanged(int width,int height) {
    int strokeW=4;
    float displayRatio=(float) width/(float)height;
    mProjM=glm::perspective(glm::radians(45.0f), displayRatio, 0.1f, 100.0f);
    mViewM=glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,-10), glm::vec3(0,1,0));
    float videoZ=-10;
    float videoH=glm::tan(glm::radians(45.0f)*0.5f)*10*2;
    float videoW=videoH*displayRatio;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    mOSDRenderer->placeGLElementsMono(videoX,videoY,videoZ,videoW,videoH,strokeW);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    //glDisable(GL_DEPTH_TEST);
    glViewport(0,0,width,height);
    glClearColor(0.0f,0,0,0.0f);
    setCPUPriority(CPU_PRIORITY_GLRENDERER_MONO,TAG);
    glLineWidth(4.0f);
}

void GLRendererMono::OnDrawFrame() {
    calculateMetrics();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    CPUFrameTime->start();
    mOSDRenderer->updateAndDrawElements(mViewM,mViewM,mProjM,false);
    CPUFrameTime->stop();
}

void GLRendererMono::setVideoDecoderFPS(float fps) {
    if(mTelemetryReceiver.get()){
        mTelemetryReceiver->get_other_osd_data()->decoder_fps=fps;
    }
    //LOGV("fps: %f",fps);
}

void GLRendererMono::OnPause() {
    //OnPause must be called after onSurfaceCreated has returned
    mTelemetryReceiver->stopReceiving();
    //if(mOSDRenderer.get()){
    mOSDRenderer->stop();
    //}
}

void GLRendererMono::calculateMetrics(){
    mFPSCalculator->tick();
    mTelemetryReceiver->get_other_osd_data()->opengl_fps=mFPSCalculator->getCurrentFPS();
    int64_t ts=getTimeMS();

    //calculate and print CPU Frame time every 2 seconds
    if(ts-lastLog>2*1000){
        lastLog=ts;
#ifdef PRINT_LOGS
        CPUFrameTime->printAvg();
#endif
        CPUFrameTime->reset();
    }
}

void GLRendererMono::setHomeLocation(double latitude, double longitude,double attitude) {
    mTelemetryReceiver->setHome(latitude,longitude,attitude);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_Activity_1MonoVidOSD_##method_name

inline jlong jptr(GLRendererMono *glRendererMono) {
    return reinterpret_cast<intptr_t>(glRendererMono);
}

inline GLRendererMono *native(jlong ptr) {
    return reinterpret_cast<GLRendererMono*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstructRenderer)
(JNIEnv *env, jclass clazz, jobject class_loader, jobject android_context,
 jlong native_gvr_api) {
    return jptr(
            new GLRendererMono(reinterpret_cast<gvr_context *>(native_gvr_api)));
}

JNI_METHOD(void, nativeDestroyRenderer)
(JNIEnv *env, jclass clazz, jlong glRendererMono) {
    delete native(glRendererMono);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererMono,jobject assetManagerJAVA) {
    LOGV("OnSurfaceCreated()");
    native(glRendererMono)->OnSurfaceCreated(env,obj,assetManagerJAVA);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h) {
    LOGV("OnSurfaceChanged() %d %d",(int)w,(int)h);
    native(glRendererMono)->OnSurfaceChanged(w,h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    //LOGV("OnDrawFrame()");
    native(glRendererMono)->OnDrawFrame();
}

JNI_METHOD(void, nativeSetVideoDecoderFPS)
(JNIEnv *env, jobject obj, jlong glRendererMono,jfloat decFPS) {
    native(glRendererMono)->setVideoDecoderFPS((float)decFPS);
}

JNI_METHOD(void, nativeOnPause)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    LOGV("OnPause()");
    native(glRendererMono)->OnPause();
}

JNI_METHOD(void, nativeSetHomeLocation)
(JNIEnv *env, jobject obj, jlong glRendererMono,jdouble latitude,jdouble longitude,jdouble attitude) {
    native(glRendererMono)->setHomeLocation((double)latitude,(double)longitude,(double)attitude);
    LOGV("setHomeLocation()");
}

}
