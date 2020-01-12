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
#include <IGLRenderer.h>
#include <IVideoFormatChanged.hpp>
#include <MatricesManager.h>

#include <GeometryBuilder/EquirectangularSphere.hpp>
#include <DistortionCorrection/VRHeadsetParams.h>

class GLRStereoNormal :  public IVideoFormatChanged {
public:
    /**
     * Create a GLRStereoNormal using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    explicit GLRStereoNormal(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,bool is360);
public:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture);
    void onSurfaceChanged(int width, int height);
    void onDrawFrame();
    //All OpenGL calls required to draw one eye (video and osd)
    void drawEye(gvr::Eye eye,bool updateOSDBetweenEyes);
    //Place the video and osd in 3D Space. Since the video ratio might change while the application is running,
    //This might be called multiple times (every time IVideoFormatChanged::videoFormatChanged==true)
    void placeGLElements();
public:
    TelemetryReceiver& mTelemetryReceiver;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const SettingsVR mSettingsVR;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTexture> mGLRenderTextureExternal= nullptr;
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    int swapColor=0;
    const bool is360;
public:
    DistortionManager distortionManager;
    VRHeadsetParams vrHeadsetParams;
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
