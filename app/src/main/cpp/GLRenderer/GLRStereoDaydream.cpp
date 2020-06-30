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
#include <GLBuffer.hpp>
#include <MatrixHelper.h>
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"
#include <gvr_util/util.h>
#include <GLHelper.hpp>

constexpr auto TAG="GLRendererDaydream";

GLRStereoDaydream::GLRStereoDaydream(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context *gvr_context,int videoSurfaceID,int screenWidthP,int screenHeightP):
        mTelemetryReceiver(telemetryReceiver),
        mSettingsVR(env,androidContext),
        mFPSCalculator("OpenGL FPS",2000)
        //,distortionManager(VDDCManager::RADIAL_CARDBOARD)
        {
    gvr_api_=gvr::GvrApi::WrapNonOwned(gvr_context);
    vrHeadsetParams.setGvrApi(gvr_api_.get());
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
    videoZ=-videoW/2.0f/(glm::tan(glm::radians(45.0f))*1.6f);
    videoZ*=1.1f;
    videoZ*=2;
    //mOSDRenderer->placeLOL(IPositionable::Rect2D(videoX,videoY,videoZ,videoW,videoH));
    //mVideoRenderer->updatePosition(videoZ,videoW,videoH,1920,1080);
}

void GLRStereoDaydream::updateBufferViewports() {
    recommended_buffer_viewports.SetToRecommendedBufferViewports();
    for(size_t eye=0;eye<2;eye++){
        recommended_buffer_viewports.GetBufferViewport(eye, &scratch_viewport);
        scratch_viewport.SetReprojection(GVR_REPROJECTION_NONE);
        buffer_viewports.SetBufferViewport(eye,scratch_viewport);
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
    mOSDRenderer=std::make_unique<OSDRenderer>(env,androidContext,mTelemetryReceiver);
    mBasicGLPrograms->text.loadTextRenderingData(env,androidContext,mOSDRenderer->settingsOSDStyle.OSD_TEXT_FONT_TYPE);

    //mVideoRenderer=std::make_unique<VideoRenderer>(VideoRenderer::VIDEO_RENDERING_MODE::RM_2D_STEREO,0,nullptr);
    placeGLElements();
    //
    float tesselatedRectSize=2.5; //6.2f
    const float offsetY=0.0f;
    /*auto tmp=ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR,{0,0,-2},{tesselatedRectSize,tesselatedRectSize},TrueColor2::BLUE);
    nColoredVertices= GLBufferHelper::createUploadGLBuffer(glBufferVC, tmp);
    tmp=ColoredGeometry::makeTessellatedColoredRectWireframe(LINE_MESH_TESSELATION_FACTOR,{0,0,-2},{tesselatedRectSize,tesselatedRectSize},TrueColor2::GREEN);
    GLBufferHelper::createUploadGLBuffer(glBufferVCX, tmp);*/
}


void GLRStereoDaydream::onSurfaceChanged(int width, int height) {
    //In GVR btw Daydream mode the onSurfaceChanged loses its importance since
    //we are rendering the scene into an off-screen buffer anyways
}

void GLRStereoDaydream::onDrawFrame() {
    //Calculate & print fps
    mFPSCalculator.tick();
    //LOGD("FPS: %f",mFPSCalculator.getCurrentFPS());
    vrHeadsetParams.updateLatestHeadSpaceFromStartSpaceRotation();

    updateBufferViewports();

    gvr::Frame frame = swap_chain->AcquireFrame();
    frame.BindBuffer(0); //0 is the 0 from createSwapChain()

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //distortionManager.updateDistortionWithIdentity();
    for(uint32_t eye=0;eye<2;eye++){
        drawEyeOSD(static_cast<gvr::Eye>(eye));
    }
    frame.Unbind();
    frame.Submit(buffer_viewports, vrHeadsetParams.GetLatestHeadSpaceFromStartSpaceRotation_());

    //
    //glClearColor(0.3f, 0.0f, 0.0f, 0.0f);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    //vrHeadsetParams.updateDistortionManager(distortionManager);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    for(int eye=0;eye<2;eye++){
        drawEyeOSDVDDC(static_cast<gvr::Eye>(eye));
    }
    GLHelper::checkGlError("GLRStereoDaydream::drawFrame");
}


void GLRStereoDaydream::drawEyeOSD(gvr::Eye eye) {
    buffer_viewports.GetBufferViewport(eye, &scratch_viewport);

    const gvr::Rectf& rect = scratch_viewport.GetSourceUv();
    int left = static_cast<int>(rect.left * framebuffer_size.width);
    int bottom = static_cast<int>(rect.bottom * framebuffer_size.width);
    int width = static_cast<int>((rect.right - rect.left) * framebuffer_size.width);
    int height = static_cast<int>((rect.top - rect.bottom) * framebuffer_size.height);
    glViewport(left, bottom, width, height);

    const gvr_rectf fov = scratch_viewport.GetSourceFov();
    const gvr::Mat4f perspective =ndk_hello_vr::PerspectiveMatrixFromView(fov, vrHeadsetParams.MIN_Z_DISTANCE,vrHeadsetParams.MAX_Z_DISTANCE);
    const auto eyeM=gvr_api_->GetEyeFromHeadMatrix(eye==0 ? GVR_LEFT_EYE : GVR_RIGHT_EYE);
    const auto rotM=gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    const auto viewM=toGLM(ndk_hello_vr::MatrixMul(eyeM,rotM));
    //const auto viewM=toGLM(eyeM);
    const auto projectionM=toGLM(perspective);
    if(eye==0){
        //mVideoRenderer->punchHole(leftEye,projection);
        //mOSDRenderer->updateAndDrawElementsGL(viewM,projectionM);
    }else{
        //mVideoRenderer->punchHole(rightEye,projection);
        //mOSDRenderer->drawElementsGL(viewM,projectionM);
    }
    glLineWidth(6.0f);
    /*mBasicGLPrograms->vc.beforeDraw(glBufferVC);
    mBasicGLPrograms->vc.draw(viewM,projectionM,0,nColoredVertices,GL_LINES);
    mBasicGLPrograms->vc.afterDraw();*/
}

void GLRStereoDaydream::drawEyeOSDVDDC(gvr::Eye eye) {
    vrHeadsetParams.setOpenGLViewport(eye);
    //distortionManager.setEye(eye==0);
    const auto rotM=toGLM(gvr_api_->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow()));
    auto viewM=vrHeadsetParams.GetEyeFromHeadMatrix(eye)*rotM;
    auto projM=vrHeadsetParams.GetProjectionMatrix(eye);

    if(eye==0){
        //mOSDRenderer->updateAndDrawElementsGL(viewM,projM);
    }else{
        //mOSDRenderer->drawElementsGL(viewM,projM);
    }

    glLineWidth(3.0f);
    /*mBasicGLPrograms->vc.beforeDraw(glBufferVCX);
    mBasicGLPrograms->vc.draw(viewM,projM,0,nColoredVertices,GL_LINES);
    mBasicGLPrograms->vc.afterDraw();*/
}



//----------------------------------------------------JAVA bindings---------------------------------------------------------------

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_xexperimental_GLRStereoDaydream_##method_name

inline jlong jptr(GLRStereoDaydream *glRenderer) {
    return reinterpret_cast<intptr_t>(glRenderer);
}
inline GLRStereoDaydream *native(jlong ptr) {
    return reinterpret_cast<GLRStereoDaydream *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstruct)
(JNIEnv *env, jobject instance,jobject androidContext,jlong telemetryReceiver, jlong native_gvr_api,jint videoSurfaceID,jint screenWidthP,jint screenHeightP) {
return jptr(new GLRStereoDaydream(env,androidContext,*reinterpret_cast<TelemetryReceiver*>(telemetryReceiver),reinterpret_cast<gvr_context *>(native_gvr_api),
        (int)videoSurfaceID,(int)screenWidthP,(int)screenHeightP));
}

JNI_METHOD(void, nativeDelete)
        (JNIEnv *env, jobject instance, jlong glRenderer) {
    delete native(glRenderer);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
        (JNIEnv *env, jobject instance, jlong glRenderer,jfloat fovY_full,jfloat ipd_full,jobject androidContext) {
    native(glRenderer)->onSurfaceCreated(env,androidContext,0);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
        (JNIEnv *env, jobject obj, jlong glRendererStereo,jint w,jint h) {
    native(glRendererStereo)->onSurfaceChanged(w, h);
}

JNI_METHOD(void, nativeOnDrawFrame)
        (JNIEnv *env, jobject obj, jlong glRenderer) {
    native(glRenderer)->onDrawFrame();
}


JNI_METHOD(void, nativeUpdateHeadsetParams)
(JNIEnv *env, jobject obj, jlong instancePointer,jobject instanceMyVrHeadsetParams) {
    const MVrHeadsetParams deviceParams=createFromJava(env, instanceMyVrHeadsetParams);
    native(instancePointer)->vrHeadsetParams.updateHeadsetParams(deviceParams);
}

}