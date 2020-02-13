/*****************************************************
 * Renders Video and OSD side by side with OpenGL
 * Rendering with and without SuperSync differs slightly
 ******************************************************/

#ifndef FPV_VR_GLRENDERERSTEREONORMAL_H
#define FPV_VR_GLRENDERERSTEREONORMAL_H

#include <glm/mat4x4.hpp>
#include <TelemetryReceiver.h>
#include "../Time/Chronometer.h"
#include <vr/gvr/capi/include/gvr_types.h>
#include "../Time/FPSCalculator.h"
#include <vr/gvr/capi/include/gvr.h>
#include <vector>
#include <Video/VideoRenderer.h>
#include <OSD/OSDRenderer.h>
#include "IVideoFormatChanged.hpp"
#include <DistortionCorrection/VRHeadsetParams.h>
#include <SettingsVR.h>

class GLRStereoNormal :  public IVideoFormatChanged {
public:
    /**
     * Create a GLRStereoNormal using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    explicit GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,int videoMode);
public:
    //not protected because unused by GLRStereoSuperSync
    void onDrawFrame();
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture);
    void onSurfaceChanged(int width, int height);
protected:
    //All OpenGL calls required to draw one eye (video and osd)
    void drawEye(gvr::Eye eye,bool updateOSDBetweenEyes);
    //Place the video and osd in 3D Space. Since the video ratio might change while the application is running,
    //This might be called multiple times (every time IVideoFormatChanged::videoFormatChanged==true)
    void placeGLElements();
protected:
    TelemetryReceiver& mTelemetryReceiver;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const SettingsVR mSettingsVR;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    //One for left and right eye each
    std::array<VertexBuffer,2> mOcclusionMesh;
    int swapColor=0;
    const VideoRenderer::VIDEO_RENDERING_MODE videoMode;
public:
    DistortionManager distortionManager;
    VRHeadsetParams vrHeadsetParams;
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
