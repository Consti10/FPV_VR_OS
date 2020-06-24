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
#include <OSD/OSDRenderer.h>
#include "IVideoFormatChanged.hpp"
#include <DistortionEngine.h>
#include <SettingsVR.h>
#include <Video/SurfaceTextureUpdate.hpp>
#include <TimeHelper.hpp>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <Extensions.hpp>
#include <queue>
#include <VrCompositorRenderer.h>
#include <Video/VideoModesHelper.hpp>

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
    void drawEye(JNIEnv* env,gvr::Eye eye,bool updateOSDBetweenEyes);
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
    int swapColor=0;
    const VideoModesHelper::VIDEO_RENDERING_MODE videoMode;
public:
    VrCompositorRenderer vrCompositorRenderer;
protected:
    SurfaceTextureUpdate mSurfaceTextureUpdate;
    AvgCalculator surfaceTextureDelay;
private:
    std::chrono::steady_clock::time_point lastRenderedFrame=std::chrono::steady_clock::now();
    // sleep until either video frame is available or timeout is reached
    void waitUntilVideoFrameAvailable(JNIEnv* env,const std::chrono::steady_clock::time_point& maxWaitTimePoint);
    void calculateFrameTimes();
    int WIDTH,HEIGHT;
    std::queue<Extensions2::SubmittedFrame> mPendingFrames;
    // SUBMIT_FRAMES: Render left and right eye as 1 frame
    // SUBMIT_HALF_FRAMES: Render left and right eye independently. Requires Front buffer rendering !
    // Doing so I can update the video texture between frames, reducing latency
    enum RENDERING_MODE{SUBMIT_FRAMES,SUBMIT_HALF_FRAMES};
    const RENDERING_MODE mRenderingMode=SUBMIT_FRAMES;
    GLuint osdFramebuffer;        // framebuffer object
    GLuint osdTexture;            // distortion texture
    const float OSD_RATIO=4.0f/3.0f;
    // Pixel maxiumum:  W 2300x1150
    const int RENDER_TEX_W=1440,RENDER_TEX_H=RENDER_TEX_W*1.0f/OSD_RATIO; //1440* 3 / 4 = 1080
    GLuint videoTextureId;
    void updatePosition(const float positionZ,const float width,const float height);
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
