

#include <vr/gvr/capi/include/gvr.h>
#include <MatrixHelper.h>
#include <AndroidThreadPrioValues.hpp>
#include <NDKThreadHelper.hpp>
#include "GLRMono.h"


constexpr auto TAG="GLRendererMono";


GLRMono::GLRMono(JNIEnv* env, jobject androidContext, TelemetryReceiver& telemetryReceiver, gvr_context* gvr_context, VideoModesHelper::VIDEO_RENDERING_MODE videoMode, bool enableOSD):
    VIDEO_MODE(videoMode),
    ENABLE_OSD(enableOSD),
    mFPSCalculator("OpenGL FPS",std::chrono::seconds(2)),
    cpuFrameTime("CPU frame time"),
    mTelemetryReceiver(telemetryReceiver),
    ENABLE_VIDEO(videoMode!=VideoModesHelper::RM_2D_MONOSCOPIC){
    if(gvr_context!= nullptr) {
        gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    }
}

void GLRMono::onSurfaceCreated(JNIEnv* env,jobject androidContext,jobject optionalSurfaceTextureHolder) {
    Extensions::initializeGL();
    NDKThreadHelper::setProcessThreadPriority(env,FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_MONO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    if(ENABLE_OSD){
        mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,mTelemetryReceiver);
    }
    //RM_2D_MONOSCOPIC is handled by the android hw composer in monoscopic 2d rendering
    if(ENABLE_VIDEO){
        assert(optionalSurfaceTextureHolder!=nullptr);
        mOptionalVideoRender.surfaceTextureUpdate=std::make_unique<SurfaceTextureUpdate>(env);
        mOptionalVideoRender.surfaceTextureUpdate->updateFromSurfaceTextureHolder(env,optionalSurfaceTextureHolder);
        mOptionalVideoRender.glProgramTextureExt=std::make_unique<GLProgramTextureExt>();
        mOptionalVideoRender.videoMesh.setData(VideoModesHelper::createMeshForMode(VIDEO_MODE,0,1,1));
    }
}

void GLRMono::onSurfaceChanged(int width, int height,float optionalVideo360FOV) {
    float displayRatio=(float) width/(float)height;
    mOptionalVideoRender.projectionMatrix=glm::perspective(glm::radians(optionalVideo360FOV), displayRatio, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    cpuFrameTime.reset();
    mOSDRenderer->onSurfaceSizeChanged(width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,width,height);
    glClearColor(0.0f,0,0,0.0f);
}

void GLRMono::onDrawFrame() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    if(ENABLE_VIDEO){
        const gvr::Mat4f tmpHeadPose = gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
        glm::mat4 tmpHeadPoseGLM=toGLM(tmpHeadPose);
        tmpHeadPoseGLM= tmpHeadPoseGLM*mOptionalVideoRender.monoForward360;
        mOptionalVideoRender.glProgramTextureExt->drawX(mOptionalVideoRender.surfaceTextureUpdate->getTextureId(),tmpHeadPoseGLM,mOptionalVideoRender.projectionMatrix,mOptionalVideoRender.videoMesh);
    }
    if(ENABLE_OSD){
        mOSDRenderer->updateAndDrawElementsGL();
    }
    mFPSCalculator.tick();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    //cpuFrameTime.printAvg(5000);
    if(ENABLE_VIDEO){
        //Reduce latency by rendering >60fps
        Extensions::eglPresentationTimeANDROID(eglGetCurrentDisplay(),eglGetCurrentSurface(EGL_DRAW),std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }
}

void GLRMono::setHomeOrientation360() {
    gvr::Mat4f tmpHeadPose=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    glm::mat4 headView=toGLM(tmpHeadPose);
    //headView=glm::toMat4(glm::quat_cast(headView));
    mOptionalVideoRender.monoForward360=mOptionalVideoRender.monoForward360*headView;
    //Reset tracking resets the rotation around the y axis, leaving everything else untouched
    gvr_api_->RecenterTracking();
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_play_1mono_GLRMono_##method_name

inline jlong jptr(GLRMono *glRendererMono) {
    return reinterpret_cast<intptr_t>(glRendererMono);
}
inline GLRMono *native(jlong ptr) {
    return reinterpret_cast<GLRMono*>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject obj,jobject androidContext,jlong telemetryReceiver,jlong nativeGvrContext,jint videoMode,jboolean enableOSD) {
    return jptr(new GLRMono(env, androidContext, *reinterpret_cast<TelemetryReceiver*>(telemetryReceiver), reinterpret_cast<gvr_context *>(nativeGvrContext), static_cast<VideoModesHelper::VIDEO_RENDERING_MODE>(videoMode), enableOSD));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    delete native(glRendererMono);
}
JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong glRendererMono,jobject androidContext,jobject optionalSurfaceTextureHolder) {
    native(glRendererMono)->onSurfaceCreated(env,androidContext,optionalSurfaceTextureHolder);
}
JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h,jfloat optionalVideo360FOV) {
    native(glRendererMono)->onSurfaceChanged(w, h,optionalVideo360FOV);
}
JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    native(glRendererMono)->onDrawFrame();
}
JNI_METHOD(void, nativeSetHomeOrientation360)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    native(glRendererMono)->setHomeOrientation360();
}
//Only called if also rendering video
JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRenderer,jint videoW,jint videoH) {
    native(glRenderer)->SetVideoRatio((int)videoW,(int)videoH);
}

}
