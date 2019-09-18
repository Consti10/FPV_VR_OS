

#include <vr/gvr/capi/include/gvr.h>
#include "GLRMono360.h"
#include "CPUPriorities.hpp"

constexpr auto TAG="GLRMono360";


GLRMono360::GLRMono360(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,bool renderOSD):
        GLRMono{env,androidContext,telemetryReceiver},
        cpuFrameTimeVidOSD("CPU frame time Vid&OSD"),
        renderOSD{renderOSD}
{
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
}

void GLRMono360::onSurfaceCreated360(JNIEnv* env,jobject androidContext,jint videoTexture) {
    GLRMono::onSurfaceCreated(env,androidContext);
    mGLProgramSpherical=std::make_unique<GLProgramSpherical>((GLuint)videoTexture);
    mVideoRenderer=std::make_unique<VideoRenderer>(mBasicGLPrograms->vc, nullptr,mSettingsVR.DEV_3D_VIDEO,mGLProgramSpherical.get());
}

void GLRMono360::onSurfaceChanged360(int width, int height) {
    GLRMono::onSurfaceChanged(width,height);
    cpuFrameTimeVidOSD.reset();
}

void GLRMono360::onDrawFrame360() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //cpuFrameTime.start();
    cpuFrameTimeVidOSD.start();
    mMatricesM.calculateNewHeadPose360(gvr_api_.get(),0);
    Matrices& worldMatrices=mMatricesM.getWorldMatrices();
    mVideoRenderer->drawVideoCanvas360(worldMatrices.monoViewTracked,worldMatrices.projection360);
    if(renderOSD){
       GLRMono::onDrawFrame(false);
    }
    cpuFrameTimeVidOSD.stop();
    cpuFrameTimeVidOSD.printAvg(5000);
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
(JNIEnv *env, jobject obj,jobject androidContext,jlong telemetryReceiver,jlong native_gvr_api,jboolean renderOSD) {
    return jptr(new GLRMono360(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),(bool)false));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    delete native(glRendererMono);
}
JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong glRendererMono,jint videoTexture,jobject androidContext) {
    native(glRendererMono)->onSurfaceCreated360(env,androidContext,videoTexture);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h) {
    native(glRendererMono)->onSurfaceChanged360(w, h);
}
JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    native(glRendererMono)->onDrawFrame360();
}

}
