

#include "GLRMono.h"
#include "CPUPriorities.hpp"


#define TAG "GLRendererMono"


GLRMono::GLRMono(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver):
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"),
        mTelemetryReceiver(telemetryReceiver),
        mSettingsVR(env,androidContext),
        mMatricesM(mSettingsVR){
}

void GLRMono::onSurfaceCreated(JNIEnv* env,jobject androidContext) {
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>();
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env,androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
}

void GLRMono::onSurfaceChanged(int width, int height) {
    float displayRatio=(float) width/(float)height;
    mMatricesM.calculateProjectionAndDefaultView(45.0f, displayRatio);
    float videoZ=-10;
    float videoH=glm::tan(glm::radians(45.0f)*0.5f)*10*2;
    float videoW=videoH*displayRatio;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    mOSDRenderer->placeGLElementsMono(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,width,height);
    glClearColor(0.0f,0,0,0.0f);
    setCPUPriority(CPU_PRIORITY_GLRENDERER_MONO,TAG);
    cpuFrameTime.reset();
    //GLProgramLine* error=nullptr;
    //error->afterDraw();
}

void GLRMono::onDrawFrame(bool clearScreen) {
    if(clearScreen){
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    cpuFrameTime.start();
    Matrices& worldMatrices=mMatricesM.getWorldMatrices();
    mOSDRenderer->updateAndDrawElementsGL(worldMatrices.eyeView,worldMatrices.projection);
    mFPSCalculator.tick();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    //cpuFrameTime.printAvg(5000);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_GLRenderer_GLRMono_##method_name

inline jlong jptr(GLRMono *glRendererMono) {
    return reinterpret_cast<intptr_t>(glRendererMono);
}
inline GLRMono *native(jlong ptr) {
    return reinterpret_cast<GLRMono*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong telemetryReceiver) {
    return jptr(new GLRMono(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver)));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    delete native(glRendererMono);
}
JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong glRendererMono,jobject androidContext) {
    native(glRendererMono)->onSurfaceCreated(env,androidContext);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h) {
    native(glRendererMono)->onSurfaceChanged(w, h);
}
JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    native(glRendererMono)->onDrawFrame(true);
}

}
