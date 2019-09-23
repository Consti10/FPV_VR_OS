
#include "GLRStereoNormal.h"
#include "jni.h"
#include "../Scene/Video/VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTextureExt.h>
#include <TelemetryReceiver.h>
#include "CPUPriorities.hpp"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define PRINT_LOGS
//#define CHANGE_SWAP_COLOR
constexpr auto TAG= "GLRendererStereo";

#include <android/choreographer.h>

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,jfloatArray undistortionData,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,bool is360):
is360(is360),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"),
        mSettingsVR(env,androidContext,undistortionData),
        mMatricesM(mSettingsVR){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
}

void GLRStereoNormal::placeGLElements(){
    if(mSettingsVR.DEV_3D_VIDEO>0){
        lastVideoFormat=lastVideoFormat/2.0f;
    }
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    float viewPortRatio=(float)ViewPortW/ViewPortH;
    float videoZ=-videoW/2.0f/viewPortRatio/glm::tan(glm::radians(MAX_FOV_USABLE_FOR_VDDC/2.0f));
    videoZ*=1.1;
    //videoZ*=0.5f;
    videoZ*=(100-mSettingsVR.VR_SceneScale)/100.0f*2;
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jint videoTexture) {
    setCPUPriority(CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    bool enableDistCorrection=mSettingsVR.VR_DistortionCorrection;
    const auto coeficients=mSettingsVR.VR_DC_UndistortionData;
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(enableDistCorrection,&coeficients);
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mGLProgramSpherical=std::make_unique<GLProgramSpherical>((GLuint)videoTexture,10,mSettingsVR.VR_DistortionCorrection,&coeficients);
    mGLRenderTextureExternal=std::make_unique<GLProgramTextureExt>((GLuint)videoTexture,mSettingsVR.VR_DistortionCorrection,&coeficients);
    mVideoRenderer=std::make_unique<VideoRenderer>(is360 ? VideoRenderer::VIDEO_RENDERING_MODE::RM_Degree360 :VideoRenderer::VIDEO_RENDERING_MODE::RM_NORMAL,
            mBasicGLPrograms->vc,mGLRenderTextureExternal.get(),mGLProgramSpherical.get(),10.0f);
}


void GLRStereoNormal::onSurfaceChanged(int width, int height) {
    ViewPortW=width/2;
    ViewPortH=height;
    placeGLElements();
    mMatricesM.calculateProjectionAndDefaultView(MAX_FOV_USABLE_FOR_VDDC,
                                                 ((float) ViewPortW) / ((float) ViewPortH));
    mMatricesM.calculateProjectionAndDefaultView360(40,((float) ViewPortW) / ((float) ViewPortH));
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    glClearColor(0,0,0,0.0F);
    //glClearColor(1,0,0,0.0f);
    cpuFrameTime.reset();
}

void GLRStereoNormal::onDrawFrame() {
#ifdef CHANGE_SWAP_COLOR
        swapColor++;
        if(swapColor>1){
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
    mFPSCalculator.tick();
    //glScissor(0,0,WindowW,WindowH);
    //glViewport(0,0,WindowW,WindowH);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    drawEyes();
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(5000);
}


void GLRStereoNormal::drawEyes() {
    mMatricesM.calculateNewHeadPoseIfNeeded(gvr_api_.get(), 16);
    mMatricesM.calculateNewHeadPose360(gvr_api_.get(),0);
    //update and draw the eyes
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
    glViewport(0,0,ViewPortW,ViewPortH);
    mVideoRenderer->drawVideoCanvas(leftEye,projection,true);
    mOSDRenderer->updateAndDrawElementsGL(leftEye,projection);
    glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    mVideoRenderer->drawVideoCanvas(rightEye,projection,false);
    mOSDRenderer->drawElementsGL(rightEye,projection);
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_PlayStereo_GLRStereoNormal_##method_name

inline jlong jptr(GLRStereoNormal *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRStereoNormal *native(jlong ptr) {
    return reinterpret_cast<GLRStereoNormal *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jfloatArray undistortionData,jlong telemetryReceiver, jlong native_gvr_api,jboolean is360) {
    return jptr(
            new GLRStereoNormal(env,androidContext,undistortionData,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),is360));
}

JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject instance, jlong glRendererStereo) {
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

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    native(glRendererStereo)->OnDrawFrame();
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->SetVideoRatio((int)videoW,(int)videoH);
}
}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079