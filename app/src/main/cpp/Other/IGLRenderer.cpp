//
// Created by Constantin on 1/21/2019.
//

#include "IGLRenderer.h"
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "IGLRenderer", __VA_ARGS__)
#define DEBUG_GL_CALLS


void IGLRenderer::OnSurfaceCreated(JNIEnv *env, jobject androidContext, jint optionalVideoTexture) {
#ifdef DEBUG_GL_CALLS
    LOGD("OnSurfaceCreated()");
#endif
    onSurfaceCreated(env,androidContext,optionalVideoTexture);
}

void IGLRenderer::OnSurfaceChanged(int width, int height) {
#ifdef DEBUG_GL_CALLS
    LOGD("OnSurfaceChanged()%dx%d",width,height);
#endif
    onSurfaceChanged(width,height);
}

void IGLRenderer::OnDrawFrame() {
#ifdef DEBUG_GL_CALLS
    //LOGD1("OnDrawFrame()");
#endif
    onDrawFrame();
}

//----------------------------------------------------JAVA bindings---------------------------------------------------------------
/*#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_GLRenderer_IGLRenderer_##method_name

inline jlong jptr(IGLRenderer *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline IGLRenderer *native(jlong ptr) {
    return reinterpret_cast<IGLRenderer *>(ptr);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jclass jclass, jlong instance,jint videoTexture,jobject androidContext) {
    native(instance)->OnSurfaceCreated(env,androidContext,videoTexture);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jclass jclass, jlong instance,jint w,jint h) {
    native(instance)->OnSurfaceChanged(w, h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jclass jclass, jlong instance) {
    native(instance)->OnDrawFrame();
}*/

