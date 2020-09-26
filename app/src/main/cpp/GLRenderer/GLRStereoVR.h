/*****************************************************
 * Renders Video and OSD side by side with OpenGL
 * Rendering with and without SuperSync differs slightly
 ******************************************************/

#ifndef FPV_VR_GLRENDERERSTEREONORMAL_H
#define FPV_VR_GLRENDERERSTEREONORMAL_H

#include <glm/mat4x4.hpp>
#include <TelemetryReceiver.h>
#include <vr/gvr/capi/include/gvr_types.h>
#include "../Time/FPSCalculator.hpp"
#include <vr/gvr/capi/include/gvr.h>
#include <vector>
#include <OSD/OSDRenderer.h>
#include "IVideoFormatChanged.hpp"
#include <VRSettings.h>
#include <SurfaceTextureUpdate.hpp>
#include <TimeHelper.hpp>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <Extensions.h>
#include <queue>
#include <VrCompositorRenderer.h>
#include <Video/VideoModesHelper.hpp>
#include <VrRenderBuffer.hpp>
#include <VrRenderBuffer2.hpp>
#include <VSYNC.h>
#include <FBRManager.h>

class GLRStereoVR : public IVideoFormatChanged {
public:
    /**
     * Create a GLRStereoVR using a given |gvr_context|.
     * @param androidContext java context
     * @param telemetryReceiver a non-owned reference to TelemetryReceiver instance
     * @param gvr_api The (non-owned) gvr_context.
     * @param videoMode The selected video mode, see @class VideoRenderer.cpp
     */
    explicit GLRStereoVR(JNIEnv* env, jobject androidContext, TelemetryReceiver& telemetryReceiver, gvr_context* gvr_context, int videoMode, jlong vsyncP);
public:
    //not protected because unused by GLRStereoSuperSync
    void onContextCreated(JNIEnv * env,jobject androidContext,int screenW,int screenH,jobject surfaceTextureHolder);
    void onDrawFrame(JNIEnv* env);
    //
    void onSecondaryContextCreated(JNIEnv* env,jobject androidContext);
    void onSecondaryContextDoWork(JNIEnv* env);
protected:
    //Place the video and osd in 3D Space. Since the video ratio might change while the application is running,
    //This might be called multiple times (every time IVideoFormatChanged::videoFormatChanged==true)
    void placeGLElements();
protected:
    TelemetryReceiver& mTelemetryReceiver;
    FPSCalculator mFPSCalculator{"VR",std::chrono::seconds(1)};
    FPSCalculator mOSDFPSCalculator{"OSD",std::chrono::seconds(1)};
    FPSCalculator mFTCalculator{"VRFT",std::chrono::seconds(1)};
    FPSCalculator mOSDFTCalculator{"OSDFT",std::chrono::seconds(1)};

    const VRSettings mSettingsVR;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    int swapColor=0;
    const VideoModesHelper::VIDEO_RENDERING_MODE videoMode;
public:
    VrCompositorRenderer vrCompositorRenderer;
    JNIEnv* currEnv=nullptr;
protected:
    SurfaceTextureUpdate mSurfaceTextureUpdate;
    AvgCalculator surfaceTextureDelay;
private:
    std::chrono::steady_clock::time_point lastRenderedFrame=std::chrono::steady_clock::now();
    // sleep until either video frame is available or timeout is reached
    void waitUntilVideoFrameAvailable(JNIEnv* env,const std::chrono::steady_clock::time_point& maxWaitTimePoint);
    void calculateFrameTimes();
    std::queue<FrameTimestamps::SubmittedFrame> mPendingFrames;
    // SUBMIT_FRAMES: Render left and right eye as 1 frame
    // SUBMIT_HALF_FRAMES: Render left and right eye independently. Requires Front buffer rendering !
    // Doing so I can update the video texture between frames, reducing latency
    enum RENDERING_MODE{SUBMIT_FRAMES,SUBMIT_HALF_FRAMES};
    const RENDERING_MODE mRenderingMode=SUBMIT_FRAMES;
    VrRenderBuffer2 osdRenderbuffer;
    const float OSD_RATIO=4.0f/3.0f;
    // Pixel maxiumum:  W 2300x1150
    const int RENDER_TEX_W=1440,RENDER_TEX_H=RENDER_TEX_W*1.0f/OSD_RATIO; //1440* 3 / 4 = 1080
    void updatePosition(const float positionZ,const float width,const float height);
    std::chrono::steady_clock::time_point lastLog=std::chrono::steady_clock::now();
    Chronometer osdCPUTime;
    AvgCalculator osdGPUTIme;
    AvgCalculator videoLatency;
    std::unique_ptr<FBRManager> mFBRManager;
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
