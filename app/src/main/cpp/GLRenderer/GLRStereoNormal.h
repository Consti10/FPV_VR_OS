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
#include <DistortionEngine.h>
#include <SettingsVR.h>
#include <Video/SurfaceTextureUpdate.hpp>

class GLRStereoNormal :  public IVideoFormatChanged {
public:
    /**
     * Create a GLRStereoNormal using a given |gvr_context|.
     * @param androidContext java context
     * @param telemetryReceiver a non-owned reference to TelemetryReceiver instance
     * @param gvr_api The (non-owned) gvr_context.
     * @param videoMode The selected video mode, see @class VideoRenderer.cpp
     */
    explicit GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,int videoMode);
public:
    //not protected because unused by GLRStereoSuperSync
    void onSurfaceCreated(JNIEnv * env,jobject androidContext,jobject videoSurfaceTexture,jint videoSurfaceTextureId);
    void onSurfaceChanged(int width, int height);
    void onDrawFrame(JNIEnv* env);
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
    VDDCManager distortionManager;
    DistortionEngine vrHeadsetParams;
protected:
    SurfaceTextureUpdate mSurfaceTextureUpdate;
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
