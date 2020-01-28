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
#include "IVideoFormatChanged.hpp"
#include <FPSCalculator.h>
#include <Video/VideoRenderer.h>
#include <DistortionCorrection/VRHeadsetParams.h>
#include <SettingsVR.h>

//Only fulfills testing purpose(s)

class GLRStereoDaydream: public IVideoFormatChanged {
public:
    GLRStereoDaydream(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,int videoSurfaceID,int screenWidthP,int screenHeightP);
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint optionalVideoTexture);
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    gvr::BufferViewportList buffer_viewports;
    gvr::BufferViewportList recommended_buffer_viewports;
    gvr::BufferViewport scratch_viewport;
    std::unique_ptr<gvr::SwapChain> swap_chain;
    gvr::Sizei framebuffer_size;
    void placeGLElements();
    void updateBufferViewports();
    void drawEyeOSD(gvr::Eye eye);
    void drawEyeOSDVDDC(gvr::Eye eye);
private:
    TelemetryReceiver& mTelemetryReceiver;
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
    DistortionManager distortionManager;
public:
    VRHeadsetParams vrHeadsetParams;
};


#endif //FPV_VR_GLRENDERERTEST_H
