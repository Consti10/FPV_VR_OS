

#include <vr/gvr/capi/include/gvr.h>
#include <MatrixHelper.h>
#include <AndroidThreadPrioValues.hpp>
#include <NDKThreadHelper.hpp>
#include "GLRMono.h"


constexpr auto TAG="GLRendererMono";


GLRMono::GLRMono(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,VideoRenderer::VIDEO_RENDERING_MODE videoMode,bool enableOSD):
    videoMode(videoMode),
    enableOSD(enableOSD),
    mFPSCalculator("OpenGL FPS",2000),
    cpuFrameTime("CPU frame time"),
    mTelemetryReceiver(telemetryReceiver){
    if(gvr_context!= nullptr) {
        gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    }
}

void GLRMono::onSurfaceCreated(JNIEnv* env,jobject androidContext,jint optionalVideoTexture) {
    NDKThreadHelper::setProcessThreadPriority(env,FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_MONO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>();
    if(enableOSD){
        mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
        mBasicGLPrograms->text.loadTextRenderingData(env,androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    }
    //RM_2D_MONOSCOPIC is handled by the android hw composer in monoscopic 2d rendering
    if(videoMode!=VideoRenderer::RM_2D_MONOSCOPIC){
        mVideoRenderer=std::make_unique<VideoRenderer>(videoMode,(GLuint)optionalVideoTexture, nullptr);
    }
}

void GLRMono::onSurfaceChanged(int width, int height,float optionalVideo360FOV) {
    float displayRatio=(float) width/(float)height;
    mOSDProjectionM=glm::perspective(glm::radians(45.0f),displayRatio,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    m360ProjectionM=glm::perspective(glm::radians(optionalVideo360FOV),displayRatio,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    cpuFrameTime.reset();
    mOSDRenderer->placeLOL(displayRatio);
    if(mVideoRenderer){
        mVideoRenderer->updatePosition(0,0,0,lastVideoWidthPx,lastVideoHeightPx);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,width,height);
    glClearColor(0.0f,0,0,0.0f);
}


void GLRMono::onDrawFrame() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    if(checkAndResetVideoFormatChanged()){
        if(mVideoRenderer){
            //mVideoRenderer->updatePosition(glm::vec3(0),0,0,lastVideoWidthPx,lastVideoHeightPx);
        }
    }
    cpuFrameTime.start();
    if(mVideoRenderer){
        if(mVideoRenderer->is360Video()){
            const gvr::Mat4f tmpHeadPose = gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
            glm::mat4 tmpHeadPoseGLM=toGLM(tmpHeadPose);
            tmpHeadPoseGLM= tmpHeadPoseGLM*monoForward360;
            mVideoRenderer->drawVideoCanvas360(tmpHeadPoseGLM,m360ProjectionM);
        }else{
            mVideoRenderer->drawVideoCanvas(mViewM,mOSDProjectionM, true);
        }
    }
    if(enableOSD){
        mOSDRenderer->updateAndDrawElementsGL(mViewM,mOSDProjectionM);
    }
    mFPSCalculator.tick();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    //cpuFrameTime.printAvg(5000);
}

void GLRMono::setHomeOrientation360() {
    gvr::Mat4f tmpHeadPose=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    glm::mat4 headView=toGLM(tmpHeadPose);
    //headView=glm::toMat4(glm::quat_cast(headView));
    monoForward360=monoForward360*headView;
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
    return jptr(new GLRMono(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(nativeGvrContext), static_cast<VideoRenderer::VIDEO_RENDERING_MODE>(videoMode),enableOSD));
}
JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    delete native(glRendererMono);
}
JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj,jlong glRendererMono,jobject androidContext,jint optionalVideoTexture) {
    native(glRendererMono)->onSurfaceCreated(env,androidContext,optionalVideoTexture);
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
