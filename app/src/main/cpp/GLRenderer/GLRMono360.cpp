

#include <vr/gvr/capi/include/gvr.h>
#include "GLRMono360.h"
#include "CPUPriorities.hpp"

constexpr auto TAG="GLRMono360";


GLRMono360::GLRMono360(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context):
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"),
        mTelemetryReceiver(telemetryReceiver),
        mSettingsVR(env,androidContext),
        mMatricesM(mSettingsVR){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
}

void GLRMono360::onSurfaceCreated(JNIEnv* env,jobject androidContext,jint optionalVideoTexture) {
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>();
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env,androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);

    mGLProgramSpherical=std::make_unique<GLProgramSpherical>((GLuint)optionalVideoTexture);
    mVideoRenderer=std::make_unique<VideoRenderer>(mBasicGLPrograms->vc, nullptr,mSettingsVR.DEV_3D_VIDEO,mGLProgramSpherical.get());
}

void GLRMono360::onSurfaceChanged(int width, int height) {
    float displayRatio=(float) width/(float)height;
    mMatricesM.calculateMatrices(45.0f,displayRatio);
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

void GLRMono360::onDrawFrame() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    mMatricesM.calculateNewHeadPose360(gvr_api_.get(),0);
    Matrices& worldMatrices=mMatricesM.getWorldMatrices();
    mVideoRenderer->drawVideoCanvas360(worldMatrices.monoViewTracked,worldMatrices.projection360);
    mOSDRenderer->updateAndDrawElementsGL(worldMatrices.eyeView,worldMatrices.projection);
    mFPSCalculator.tick();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    //cpuFrameTime.printAvg(5000);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_GLRenderer_GLRMono360_##method_name

inline jlong jptr(GLRMono360 *glRendererMono) {
    return reinterpret_cast<intptr_t>(glRendererMono);
}
inline GLRMono360 *native(jlong ptr) {
    return reinterpret_cast<GLRMono360*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong telemetryReceiver,jlong native_gvr_api) {
    return jptr(new GLRMono360(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api)));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    delete native(glRendererMono);
}
JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong glRendererMono,jint videoTexture,jobject androidContext) {
    native(glRendererMono)->OnSurfaceCreated(env,androidContext,videoTexture);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h) {
    native(glRendererMono)->OnSurfaceChanged(w, h);
}
JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    native(glRendererMono)->OnDrawFrame();
}

}
