//
// Created by Constantin on 29.03.2018.
//

#include "GLRStereoDaydream.h"
#include "jni.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTexture.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <memory>
#include "CPUPriorities.hpp"

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include "Helper/GLHelper.hpp"

#define TAG "GLRendererDaydream"

GLRStereoDaydream::GLRStereoDaydream(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,int videoSurfaceID):
        mTelemetryReceiver(telemetryReceiver),
        mSettingsVR(env,androidContext),
        mMatricesM(mSettingsVR),
        mFPSCalculator("OpenGL FPS",2000){
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    this->videoSurfaceID=videoSurfaceID;
    //----------
    buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    recommended_buffer_viewports = gvr_api_->CreateEmptyBufferViewportList();
    scratch_viewport = gvr_api_->CreateBufferViewport();
}

void GLRStereoDaydream::placeGLElements() {
    float videoW=10;
    float videoH=videoW*1.0f/1.7777f;
    float videoX=-videoW/2.0f;
    float videoY=-videoH/2.0f;
    videoZ=-videoW/2.0f/glm::tan(glm::radians(60.0f/2.0f));
    videoZ*=1.1f;
    videoZ*=2;
    mOSDRenderer->placeGLElementsStereo(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    mVideoRenderer->setWorldPosition(videoX,videoY,videoZ,videoW,videoH);
}

void GLRStereoDaydream::updateBufferViewports() {
    Matrices& t=mMatricesM.getWorldMatrices();
    //recommended_buffer_viewports.SetToRecommendedBufferViewports();
    //First the view ports for the video, handled by the async reprojection
    /*for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        //scratch_viewport.SetSourceBufferIndex(GVR_BUFFER_INDEX_EXTERNAL_SURFACE);
        //scratch_viewport.SetExternalSurfaceId(videoSurfaceID);
        //scratch_viewport.SetExternalSurfaceId(GVR_EXTERNAL_SURFACE_ID_NONE);

        glm::mat4x4 glmM=glm::mat4x4(1);
        glmM=glm::scale(glmM,glm::vec3(1.6f,0.9f,1.0f));
        glmM=glm::translate(glmM,glm::vec3(0.0f,0.0f,-4));

        gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
        target_time.monotonic_system_time_nanos+=10*1000*1000;
        glm::mat4x4 view=toGLM(gvr_api_->GetHeadSpaceFromStartSpaceRotation(target_time));
        glmM*=(eye==0 ? t.leftEyeView : t.rightEyeView);
        gvr::Mat4f gvrM=toGVR(glmM);
        scratch_viewport.SetTransform(gvrM);
        gvr::Rectf fov={50,50,30,30};
        scratch_viewport.SetSourceFov(fov);
        //auto b=scratch_viewport.GetSourceUv();
        //LOGDX("%f %f",b.bottom,b.top);
        //LOGDX("%f %f",b.left,b.right);
        gvr::Rectf uv={0,1,0,1}; //sample the same, full frame for left / right eye (video is not stereo)
        scratch_viewport.SetSourceUv(uv);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
    }*/
    recommended_buffer_viewports.SetToRecommendedBufferViewports();
    //
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        //gvr::Rectf fov={45,45,45,45};
        //gvr::Rectf fov={33.15,33.15,33.15,33.15};
        //scratch_viewport.SetSourceFov(fov);
        //scratch_viewport.Set
        scratch_viewport.SetExternalSurfaceId(videoSurfaceID);
        scratch_viewport.SetSourceBufferIndex(GVR_BUFFER_INDEX_EXTERNAL_SURFACE);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        //scratch_viewport.SetTransform()
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
    }
    //
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        //gvr::Rectf fov={45,45,45,45};
        //gvr::Rectf fov={33.15,33.15,33.15,33.15};
        //scratch_viewport.SetSourceFov(fov);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye+2,scratch_viewport);
    }
}

void GLRStereoDaydream::onSurfaceCreated(JNIEnv * env,jobject androidContext,jint optionalVideoTexture) {
    gvr_api_->InitializeGl();
    std::vector<gvr::BufferSpec> specs;
    specs.push_back(gvr_api_->CreateBufferSpec());
    framebuffer_size = gvr_api_->GetMaximumEffectiveRenderTargetSize();
    specs[0].SetSize(framebuffer_size);
    specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
    specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
    swap_chain = std::make_unique<gvr::SwapChain>(gvr_api_->CreateSwapChain(specs));

    mBasicGLPrograms=std::make_unique<BasicGLPrograms>();
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,*mBasicGLPrograms,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env,androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);
    mVideoRenderer=std::make_unique<VideoRenderer>(VideoRenderer::VIDEO_RENDERING_MODE::RM_NORMAL,mBasicGLPrograms->vc);

    mMatricesM.calculateProjectionAndDefaultView(headset_fovY_full, framebuffer_size.width / 2.0f /
                                                                    framebuffer_size.height);
    placeGLElements();
}

void GLRStereoDaydream::onSurfaceChanged(int width, int height) {
    //In GVR btw Daydream mode the onSurfaceChanged loses its importance since
    //we are rendering the scene into an off-screen buffer anyways
}

void GLRStereoDaydream::onDrawFrame() {
    //Calculate & print fps
    mFPSCalculator.tick();
    LOGD("FPS: %f",mFPSCalculator.getCurrentFPS());

    mMatricesM.calculateNewHeadPoseIfNeeded(gvr_api_.get(), 16);
    Matrices& worldMatrices=mMatricesM.getWorldMatrices();

    updateBufferViewports();

    gvr::Frame frame = swap_chain->AcquireFrame();
    frame.BindBuffer(0); //0 is the 0 from createSwapChain()

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //Video is handled by the async reprojection surface
    for(uint32_t eye=0;eye<2;eye++){
        drawEyeOSD(eye, worldMatrices);
    }
    frame.Unbind();
    frame.Submit(buffer_viewports, worldMatrices.lastHeadPos);

    GLHelper::checkGlError("GLRStereoDaydream::drawFrame");
}


void GLRStereoDaydream::drawEyeOSD(uint32_t eye, Matrices &worldMatrices) {
    buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
    const gvr::Rectf& rect = scratch_viewport.GetSourceUv();
    int left = static_cast<int>(rect.left * framebuffer_size.width);
    int bottom = static_cast<int>(rect.bottom * framebuffer_size.width);
    int width = static_cast<int>((rect.right - rect.left) * framebuffer_size.width);
    int height = static_cast<int>((rect.top - rect.bottom) * framebuffer_size.height);
    glViewport(left, bottom, width, height);

    glm::mat4x4 leftEye,rightEye,projection;
    leftEye=worldMatrices.leftEyeView;
    rightEye=worldMatrices.rightEyeView;
    projection=worldMatrices.projection;

    if(eye==0){
        //mVideoRenderer->punchHole(leftEye,projection);
        mOSDRenderer->updateAndDrawElementsGL(leftEye,projection);
    }else{
        //mVideoRenderer->punchHole(rightEye,projection);
        mOSDRenderer->drawElementsGL(rightEye,projection);
    }

}





//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_PlayStereo_GLRStereoDaydream_##method_name

inline jlong jptr(GLRStereoDaydream *glRenderer) {
    return reinterpret_cast<intptr_t>(glRenderer);
}
inline GLRStereoDaydream *native(jlong ptr) {
    return reinterpret_cast<GLRStereoDaydream *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jlong telemetryReceiver, jlong native_gvr_api,jint videoSurfaceID) {
return jptr(new GLRStereoDaydream(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),(int)videoSurfaceID));
}

JNI_METHOD(void, nativeDelete)
        (JNIEnv *env, jobject instance, jlong glRenderer) {
    delete native(glRenderer);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
        (JNIEnv *env, jobject instance, jlong glRenderer,jfloat fovY_full,jfloat ipd_full,jobject androidContext) {
    native(glRenderer)->OnSurfaceCreated(env,androidContext,0);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
        (JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->OnSurfaceChanged(w, h);
}

JNI_METHOD(void, nativeOnDrawFrame)
        (JNIEnv *env, jobject obj, jlong glRenderer) {
    native(glRenderer)->OnDrawFrame();
}


}