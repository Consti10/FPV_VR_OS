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

class GLRStereoNormal : public IGLRenderer, public IVideoFormatChanged {
public:
    /**
     * Create a GLRStereoNormal using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    explicit GLRStereoNormal(JNIEnv* env,jobject androidContext,jfloatArray undistortionData,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,bool is360);
private:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture) override;
    void onSurfaceChanged(int width, int height)override ;
    void onDrawFrame()override ;
    void drawEyes();
    void placeGLElements();
private:
    TelemetryReceiver& mTelemetryReceiver;
    MatricesManager mMatricesM;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const SettingsVR mSettingsVR;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTextureExt> mGLRenderTextureExternal= nullptr;
    std::unique_ptr<GLProgramSpherical> mGLProgramSpherical= nullptr;
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    int ViewPortW=0,ViewPortH=0;
    int swapColor=0;
    const float MAX_FOV_USABLE_FOR_VDDC=100;
    const bool is360;
};


#endif //FPV_VR_GLRENDERERSTEREONORMAL_H
