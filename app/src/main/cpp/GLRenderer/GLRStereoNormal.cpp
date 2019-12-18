
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

GLRStereoNormal::GLRStereoNormal(JNIEnv* env,jobject androidContext,jfloatArray undistortionData,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,bool is360):
is360(is360),
distortionManager(DistortionManager::RADIAL_2),
        mTelemetryReceiver(telemetryReceiver),
        mFPSCalculator("OpenGL FPS",2000),
        cpuFrameTime("CPU frame time"),
        mSettingsVR(env,androidContext,undistortionData,gvr_context),
        mMatricesM(mSettingsVR){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
}

void GLRStereoNormal::placeGLElements(){
    float videoW=10;
    float videoH=videoW*1.0f/lastVideoFormat;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    float viewPortRatio=(float)ViewPortW/ViewPortH;
    float videoZ=-videoW/2.0f/viewPortRatio/glm::tan(glm::radians(MAX_FOV_USABLE_FOR_VDDC/2.0f));
    videoZ*=1.1f;
    //videoZ*=0.5f;
    videoZ*=(100-mSettingsVR.VR_SceneScale)/100.0f*2;
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRStereoNormal::onSurfaceCreated(JNIEnv * env,jobject androidContext,jint videoTexture) {
    setCPUPriority(CPU_PRIORITY_GLRENDERER_STEREO,TAG);
    //Once we have an OpenGL context, we can create our OpenGL world object instances. Note the use of shared btw. unique pointers:
    //If the phone does not preserve the OpenGL context when paused, OnSurfaceCreated might be called multiple times
    mBasicGLPrograms=std::make_unique<BasicGLPrograms>(&distortionManager);
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env, androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mGLProgramSpherical=std::make_unique<GLProgramSpherical>(10,&distortionManager);
    mGLRenderTextureExternal=std::make_unique<GLProgramTexture>(true,&distortionManager);
    mVideoRenderer=std::make_unique<VideoRenderer>(is360 ? VideoRenderer::VIDEO_RENDERING_MODE::RM_Degree360 :VideoRenderer::VIDEO_RENDERING_MODE::RM_NORMAL,
            (GLuint)videoTexture,mBasicGLPrograms->vc,mGLRenderTextureExternal.get(),mGLProgramSpherical.get(),10.0f);
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
    glm::mat4x4 view,projection;
    /*if(mMatricesM.settingsVR.GHT_MODE==MatricesManager::MODE_1PP){
        Matrices& worldMatrices=mMatricesM.getWorldMatrices();
        leftEye=worldMatrices.leftEyeViewTracked;
        rightEye=worldMatrices.rightEyeViewTracked;
        projection=worldMatrices.projection;
    }else{
        Matrices& worldMatrices=mMatricesM.getWorldMatrices();
        leftEye=worldMatrices.leftEyeView;
        rightEye=worldMatrices.rightEyeView;
        projection=worldMatrices.projection;
    }*/
    const auto rotation=toGLM(gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow()));
    int eye=0;
    view=mViewM[eye]*rotation;
    projection=mProjectionM[eye];
    glViewport(0,0,ViewPortW,ViewPortH);
    distortionManager.leftEye=true;
    mVideoRenderer->drawVideoCanvas(view,projection,true);
    mOSDRenderer->updateAndDrawElementsGL(view,projection);
    eye=1;
    view=mViewM[eye]*rotation;
    projection=mProjectionM[eye];
    glViewport(ViewPortW,0,ViewPortW,ViewPortH);
    distortionManager.leftEye=false;
    mVideoRenderer->drawVideoCanvas(view,projection,false);
    mOSDRenderer->drawElementsGL(view,projection);
}


void GLRStereoNormal::updateHeadsetParams(const MDeviceParams& mDP) {
    LOGD("%s",MLensDistortion::MDeviceParamsAsString(mDP).c_str());
    auto polynomialRadialDistortion=MPolynomialRadialDistortion(mDP.radial_distortion_params);
    //auto polynomialRadialDistortion=MPolynomialRadialDistortion({0.441, 0.156});

    const auto GetYEyeOffsetMeters= MLensDistortion::GetYEyeOffsetMeters(mDP.vertical_alignment,
                                                                         mDP.tray_to_lens_distance,
                                                                         mDP.screen_height_meters);
    const auto fovLeft= MLensDistortion::CalculateFov(mDP.device_fov_left, GetYEyeOffsetMeters,
                                                      mDP.screen_to_lens_distance,
                                                      mDP.inter_lens_distance,
                                                      polynomialRadialDistortion,
                                                      mDP.screen_width_meters, mDP.screen_height_meters);
    const auto fovRight=MLensDistortion::reverseFOV(fovLeft);

    std::array<MLensDistortion::ViewportParams,2> screen_params;
    std::array<MLensDistortion::ViewportParams,2> texture_params;

    MLensDistortion::CalculateViewportParameters_NDC(0, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovLeft,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[0], &texture_params[0]);
    MLensDistortion::CalculateViewportParameters_NDC(1, GetYEyeOffsetMeters,
                                                     mDP.screen_to_lens_distance,
                                                     mDP.inter_lens_distance, fovRight,
                                                     mDP.screen_width_meters, mDP.screen_height_meters,
                                                     &screen_params[1], &texture_params[1]);
    float MAX_RAD_SQ=1.0f;
    bool done=false;
    while(MAX_RAD_SQ<2.0f && !done){
        const auto inverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);
        LOGD("Max Rad Sq%f",MAX_RAD_SQ);
        for(float r=0;r<MAX_RAD_SQ;r+=0.01f) {
            const float deviation = MPolynomialRadialDistortion::calculateDeviation(r,polynomialRadialDistortion,inverse);
            //LOGD("r %f | Deviation %f",r,deviation);
            if (deviation > 0.001f) {
                done = true;
                MAX_RAD_SQ-= 0.01f;
                break;
            }
        }
        MAX_RAD_SQ+=0.01f;
    }
    LOGD("Max Rad Sq%f",MAX_RAD_SQ);
    const auto mInverse=polynomialRadialDistortion.getApproximateInverseDistortion(MAX_RAD_SQ,DistortionManager::N_RADIAL_UNDISTORTION_COEFICIENTS);

    distortionManager.updateDistortion(mInverse,MAX_RAD_SQ,screen_params,texture_params);

    mProjectionM[0]=perspective(fovLeft,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    mProjectionM[1]=perspective(fovRight,MIN_Z_DISTANCE,MAX_Z_DISTANCE);
    const float inter_lens_distance=mDP.inter_lens_distance;
    mViewM[0]=glm::translate(glm::mat4(1.0f),glm::vec3(inter_lens_distance*0.5f,0,0));
    mViewM[1]=glm::translate(glm::mat4(1.0f),glm::vec3(-inter_lens_distance*0.5f,0,0));
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


JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong glRendererStereo,
jfloat screen_width_meters,
jfloat screen_height_meters,
jfloat screen_to_lens_distance,
jfloat inter_lens_distance,
jint vertical_alignment,
jfloat tray_to_lens_distance,
jfloatArray device_fov_left,
jfloatArray radial_distortion_params
        ) {
    std::array<float,4> device_fov_left1{40,40,40,40};
    std::vector<float> radial_distortion_params1{0,0};

    jfloat *arrayP=env->GetFloatArrayElements(device_fov_left, nullptr);
    std::memcpy(device_fov_left1.data(),&arrayP[0],4*sizeof(float));
    env->ReleaseFloatArrayElements(device_fov_left,arrayP,0);
    arrayP=env->GetFloatArrayElements(radial_distortion_params, nullptr);
    std::memcpy(radial_distortion_params1.data(),&arrayP[0],2*sizeof(float));
    env->ReleaseFloatArrayElements(radial_distortion_params,arrayP,0);

    const MDeviceParams deviceParams{screen_width_meters,screen_height_meters,screen_to_lens_distance,inter_lens_distance,vertical_alignment,tray_to_lens_distance,
                                     device_fov_left1,radial_distortion_params1};
    native(glRendererStereo)->updateHeadsetParams(deviceParams);
}


}

//gvr_vec2f uv_in{0.5f,0.5f};
//    gvr_vec2f uv_out[3];
//    gvr_compute_distorted_point(gvr_api_->GetContext(),0,uv_in,uv_out);
//    LOGD("UV out: %f %f",uv_out[1].x,uv_out[1].y);
//    //0==0.466929 0.456079
//    //1==0.466929 0.456079