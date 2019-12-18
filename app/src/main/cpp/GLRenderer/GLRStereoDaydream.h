//
// Created by Constantin on 29.03.2018.
//

#ifndef FPV_VR_GLRENDERERTEST_H
#define FPV_VR_GLRENDERERTEST_H


#include <glm/mat4x4.hpp>
#include <TelemetryReceiver.h>
#include "vr/gvr/capi/include/gvr_types.h"
#include "vr/gvr/capi/include/gvr.h"
#include <vector>
#include <OSD/OSDRenderer.h>
#include <IGLRenderer.h>
#include <IVideoFormatChanged.hpp>
#include <MatricesManager.h>
#include <FPSCalculator.h>
#include <Video/VideoRenderer.h>

class GLRStereoDaydream: public IGLRenderer,public IVideoFormatChanged {
public:
    GLRStereoDaydream(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,int videoSurfaceID,int screenWidthP,int screenHeightP);
    void updateHeadsetParams(const MDeviceParams& deviceParams);
private:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint optionalVideoTexture)override;
    void onSurfaceChanged(int width, int height)override ;
    void onDrawFrame()override;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    //-----
    gvr::BufferViewportList buffer_viewports;
    gvr::BufferViewportList recommended_buffer_viewports;
    gvr::BufferViewport scratch_viewport;
    std::unique_ptr<gvr::SwapChain> swap_chain;
    gvr::Sizei framebuffer_size;
    //----------
private:
    void placeGLElements();
    void updateBufferViewports();
    void drawEyeOSD(uint32_t eye, Matrices &worldMatrices);
    void drawEyeOSDVDDC(uint32_t eye);
private:
    TelemetryReceiver& mTelemetryReceiver;
    MatricesManager mMatricesM;
    const SettingsVR mSettingsVR;
    FPSCalculator mFPSCalculator;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;

    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    int videoSurfaceID=GVR_EXTERNAL_SURFACE_ID_NONE;
    float headset_fovY_full=30;
    float headset_ipd_full=0.2f;
    float videoZ;

    static constexpr int LINE_MESH_TESSELATION_FACTOR=10;
    GLuint glBufferVC;
    GLuint glBufferVCX;
    int nColoredVertices;
private:
    const int screenWidthP;
    const int screenHeightP;
private:
    DistortionManager distortionManager;
    glm::mat4 mViewM[2];
    glm::mat4 mProjectionM[2];
    static constexpr float MIN_Z_DISTANCE=0.1f;
    static constexpr float MAX_Z_DISTANCE=100.0f;
private:
    std::array<MLensDistortion::ViewportParams,2> screen_params;
    std::array<MLensDistortion::ViewportParams,2> texture_params;
    MPolynomialRadialDistortion mInverse{{0}};
    float MAX_RAD_SQ;
};


#endif //FPV_VR_GLRENDERERTEST_H
