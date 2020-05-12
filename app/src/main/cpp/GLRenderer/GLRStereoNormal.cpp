
#include "GLRStereoNormal.h"
#include "jni.h"
#include "../Scene/Video/VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTexture.h>
#include <TelemetryReceiver.h>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define PRINT_LOGS
//#define CHANGE_SWAP_COLOR
constexpr auto TAG= "GLRendererStereo";

#include <android/choreographer.h>
#include <MatrixHelper.h>
#include <CardboardViewportOcclusion.hpp>
#include <CPUPriority.hpp>

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,const int videoMode):
videoMode(static_cast<VideoRenderer::VIDEO_RENDERING_MODE>(videoMode)),mSettingsVR(env,androidContext),
distortionManager(mSettingsVR.VR_DISTORTION_CORRECTION_MODE==0 ? VDDCManager::NONE : VDDCManager::RADIAL_CARDBOARD),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
}

void GLRStereoNormal::placeGLElements(){
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    //The video width defaults to 10(cm). Calculate tze z value such that the video fills a FOV
    //of exactly DEFAULT_FOV_FILLED_BY_SCENE
    float videoZ=-videoW/2.0f/glm::tan(glm::radians(SettingsVR::DEFAULT_FOV_FILLED_BY_SCENE/2.0f));
    videoZ*=1/(mSettingsVR.VR_SCENE_SCALE_PERCENTAGE/100.0f);
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->updatePosition(videoZ,videoW,videoH,lastVideoWidthPx,lastVideoHeightPx);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jint videoTexture) {
    CPUPriority::setCPUPriority(FPV_VR_PRIORITY::CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(&distortionManager);
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mVideoRenderer=std::make_unique<VideoRenderer>(videoMode,(GLuint)videoTexture,&distortionManager);
    const auto color=TrueColor2::BLACK;
    CardboardViewportOcclusion::uploadOcclusionMeshLeftRight(vrHeadsetParams,color,mOcclusionMesh);
}

void GLRStereoNormal::onSurfaceChanged(int width, int height) {
    placeGLElements();
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,0.0F);
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
    if(checkAndResetVideoFormatChanged()){
        placeGLElements();
    }
    mFPSCalculator.tick();
    vrHeadsetParams.updateLatestHeadSpaceFromStartSpaceRotation();
    //start rendering the frame
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    cpuFrameTime.start();
    drawEye(GVR_LEFT_EYE,false);
    drawEye(GVR_RIGHT_EYE, false);
    mTelemetryReceiver.setOpenGLFPS(mFPSCalculator.getCurrentFPS());
    cpuFrameTime.stop();
    cpuFrameTime.printAvg(5000);
}


void GLRStereoNormal::drawEye(gvr::Eye eye,bool updateOSDBetweenEyes){
    distortionManager.setEye(eye==GVR_LEFT_EYE);
    vrHeadsetParams.setOpenGLViewport(eye);
    //Now draw
    const auto rotation=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation();
    glm::mat4 viewVideo;
    glm::mat4 viewOSD;
    if(mVideoRenderer->is360Video()){
        //When rendering 360° video,always fully track head rotation for video, optionally lock OSD
        viewVideo= vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotation;
        viewOSD= mSettingsVR.GHT_OSD_FIXED_TO_HEAD ? vrHeadsetParams.GetEyeFromHeadMatrix(eye):
                vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotation;
    }else{
        //Else, track video only if head tracking is enabled, lock OSD if requested
        const auto rotationWithAxesDisabled=removeRotationAroundSpecificAxes(rotation,mSettingsVR.GHT_X,mSettingsVR.GHT_Y,mSettingsVR.GHT_Z);
        viewVideo=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotationWithAxesDisabled;
        viewOSD= mSettingsVR.GHT_OSD_FIXED_TO_HEAD ? vrHeadsetParams.GetEyeFromHeadMatrix(eye)
                                                   : vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotationWithAxesDisabled;
    }
    const glm::mat4 projection=vrHeadsetParams.GetProjectionMatrix(eye);
    mVideoRenderer->drawVideoCanvas(viewVideo,projection,eye==GVR_LEFT_EYE);
    if(eye==GVR_LEFT_EYE || updateOSDBetweenEyes){
        mOSDRenderer->updateAndDrawElementsGL(viewOSD,projection);
    }else{
        mOSDRenderer->drawElementsGL(viewOSD,projection);
    }
    //Render the mesh that occludes everything except the part actually visible inside the headset
    if(mSettingsVR.VR_DISTORTION_CORRECTION_MODE!=0){
        int idx=eye==GVR_LEFT_EYE ? 0 : 1;
        mBasicGLPrograms->vc2D.drawX(glm::mat4(1.0f),glm::mat4(1.0f),mOcclusionMesh[idx]);
    }
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------
#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_play_1stereo_GLRStereoNormal_##method_name

inline jlong jptr(GLRStereoNormal *glRendererStereo) {
    return reinterpret_cast<intptr_t>(glRendererStereo);
}
inline GLRStereoNormal *native(jlong ptr) {
    return reinterpret_cast<GLRStereoNormal *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jlong telemetryReceiver, jlong native_gvr_api,jint videoMode) {
        return jptr(
            new GLRStereoNormal(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),videoMode));
}

JNI_METHOD(void, nativeDelete)
(JNIEnv *env, jobject instance, jlong glRendererStereo) {
    delete native(glRendererStereo);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoTexture,jobject androidContext) {
    native(glRendererStereo)->onSurfaceCreated(env,androidContext,videoTexture);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->onSurfaceChanged(w, h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererStereo) {
    native(glRendererStereo)->onDrawFrame();
}

JNI_METHOD(void, nativeOnVideoRatioChanged)
(JNIEnv *env, jobject obj, jlong glRendererStereo,jint videoW,jint videoH) {
    native(glRendererStereo)->SetVideoRatio((int)videoW,(int)videoH);
}


JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrHeadsetParams.updateHeadsetParams(deviceParams);
    native(instancePointer)->vrHeadsetParams.updateDistortionManager(native(instancePointer)->distortionManager);
}

}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079