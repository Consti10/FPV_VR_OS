
#include "GLRStereoNormal.h"
#include "jni.h"
#include "../Scene/Video/VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTexture.h>
#include <TelemetryReceiver.h>
#include "CPUPriorities.hpp"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define PRINT_LOGS
//#define CHANGE_SWAP_COLOR
constexpr auto TAG= "GLRendererStereo";

#include <android/choreographer.h>
#include <MatrixHelper.h>
#include <GeometryBuilder/CardboardViewportOcclusion.h>

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,const int videoMode):
videoMode(videoMode),mSettingsVR(env,androidContext),
distortionManager(mSettingsVR.VR_DISTORTION_CORRECTION_MODE==0 ? DistortionManager::NONE : DistortionManager::RADIAL_CARDBOARD),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
}

void GLRStereoNormal::placeGLElements(){
    float videoW=10;
    float videoH=videoW*1.0f/1.77777f;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    //The video width defaults to 10(cm). Calculate tze z value such that the video fills a FOV
    //of exactly DEFAULT_FOV_FILLED_BY_SCENE
    float videoZ=-videoW/2.0f/glm::tan(glm::radians(SettingsVR::DEFAULT_FOV_FILLED_BY_SCENE/2.0f));
    videoZ*=1/(mSettingsVR.VR_SCENE_SCALE_PERCENTAGE/100.0f);
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->updatePosition(glm::vec3(videoX,videoY,videoZ),videoW,videoH,lastVideoWidthPx,lastVideoHeightPx);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jint videoTexture) {
    setCPUPriority(CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(&distortionManager);
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mGLRenderTextureExternal=std::make_unique<GLProgramTextureExt>(&distortionManager);
    mVideoRenderer=std::make_unique<VideoRenderer>(static_cast<VideoRenderer::VIDEO_RENDERING_MODE>(videoMode),
            (GLuint)videoTexture,mBasicGLPrograms->vc,mGLRenderTextureExternal.get());
    const TrueColor color=Color::BLACK;
    mOcclusionMesh[0].initializeAndUploadGL(CardboardViewportOcclusion::makeMesh(vrHeadsetParams,0,color));
    mOcclusionMesh[1].initializeAndUploadGL(CardboardViewportOcclusion::makeMesh(vrHeadsetParams,1,color));
}

void GLRStereoNormal::onSurfaceChanged(int width, int height) {
    placeGLElements();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
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
    distortionManager.leftEye=eye==GVR_LEFT_EYE;
    vrHeadsetParams.setOpenGLViewport(eye);
    //Now draw
    auto rotation=vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation();
    //Always enable head tracking if 360 degree
    if(videoMode!=2){
        rotation=removeRotationAroundSpecificAxes(rotation,mSettingsVR.GHT_X,mSettingsVR.GHT_Y,mSettingsVR.GHT_Z);
    }
    glm::mat4 view=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotation;
    glm::mat4 projection=vrHeadsetParams.GetProjectionMatrix(eye);
    mVideoRenderer->drawVideoCanvas(view,projection,eye==GVR_LEFT_EYE);
    if(eye==GVR_LEFT_EYE || updateOSDBetweenEyes){
        mOSDRenderer->updateAndDrawElementsGL(view,projection);
    }else{
        mOSDRenderer->drawElementsGL(view,projection);
    }
    //Render the mesh that occludes everything except the part actually visible inside the headset
    if(mSettingsVR.VR_DISTORTION_CORRECTION_MODE!=0){
        int idx=eye==GVR_LEFT_EYE ? 0 : 1;
        mBasicGLPrograms->vc2D.beforeDraw(mOcclusionMesh[idx].vertexB);
        mBasicGLPrograms->vc2D.draw(glm::mat4(1.0f),glm::mat4(1.0f),0,mOcclusionMesh[idx].nVertices,GL_TRIANGLE_STRIP);
        mBasicGLPrograms->vc2D.afterDraw();
    }
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
(JNIEnv *env, jobject obj, jlong instance,
 jfloat screen_width_meters,
 jfloat screen_height_meters,
 jfloat screen_to_lens_distance,
 jfloat inter_lens_distance,
 jint vertical_alignment,
 jfloat tray_to_lens_distance,
 jfloatArray device_fov_left,
 jfloatArray radial_distortion_params,
 jint screenWidthP,jint screenHeightP
) {
    std::array<float,4> device_fov_left1{};
    std::vector<float> radial_distortion_params1(2);

    jfloat *arrayP=env->GetFloatArrayElements(device_fov_left, nullptr);
    std::memcpy(device_fov_left1.data(),&arrayP[0],4*sizeof(float));
    env->ReleaseFloatArrayElements(device_fov_left,arrayP,0);
    arrayP=env->GetFloatArrayElements(radial_distortion_params, nullptr);
    std::memcpy(radial_distortion_params1.data(),&arrayP[0],2*sizeof(float));
    env->ReleaseFloatArrayElements(radial_distortion_params,arrayP,0);

    const MDeviceParams deviceParams{screen_width_meters,screen_height_meters,screen_to_lens_distance,inter_lens_distance,vertical_alignment,tray_to_lens_distance,
                                     device_fov_left1,radial_distortion_params1};
    native(instance)->vrHeadsetParams.updateHeadsetParams(deviceParams,screenWidthP,screenHeightP);
    native(instance)->vrHeadsetParams.updateDistortionManager(native(instance)->distortionManager);
}

}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079