/*****************************************************
 * Renders Video and OSD side by side with OpenGL
 ******************************************************/

#ifndef FPV_VR_GLRSUPERSYNC_H
#define FPV_VR_GLRSUPERSYNC_H

#include "jni.h"
#include "../Scene/Video/VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTextureExt.h>
#include <TelemetryReceiver.h>
#include <EGL/egl.h>
#include <cinttypes>
#include "Extensions.hpp"
#include "CPUPriorities.hpp"
#include <sys/resource.h>
#include <FBRManager.h>
#include "vr/gvr/capi/include/gvr_types.h"
#include <vector>
#include <OSD/OSDRenderer.h>
#include <IGLRenderer.h>
#include <IVideoFormatChanged.hpp>
#include <VRFrameCPUChronometer.h>
#include <MatricesManager.h>

class GLRStereoSuperSync : public IGLRenderer,public IVideoFormatChanged{
public:
    /**
     * Create a GLRenderer Stereo SuperSync using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    GLRStereoSuperSync(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,bool qcomTiledRenderingAvailable,bool reusableSyncAvailable,bool is360);
    /**
    * Draw the Video and transparent OSD scene, synchronized with the VSYNC, directly into the Front Buffer
     * This has to be called on the GL thread.
     * WARNING: does not return until exitSuperSyncLoop is called. Basically it blocks the GL thread.
    */
    void enterSuperSyncLoop(JNIEnv * env, jobject obj,jobject surfaceTexture,int exclusiveVRCore);
    /**
     * Exit the SuperSync loop. Since the super sync loop blocks the GLThread, this has to be called from another thread, e.g the UI thread
     * called by the UI's onPause().
     */
    void exitSuperSyncLoop();
    /**
     * Pass trough the last VSYNC from java
     * @param lastVSYNC last reported vsync, in ns
     */
    void setLastVSYNC(int64_t lastVSYNC);
private:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture) override ;
    void onSurfaceChanged(int width, int height)override;
    void onDrawFrame()override{};//unused. using FBRManager, lambda and drawEye() instead
    void drawEye(bool whichEye);
    /**
     * Called by the SuperSync manager
     * @param env
     * @param whichEye left/right eye
     * @param offsetNS  time since eye event
     */
    void renderNewEyeCallback(JNIEnv* env,bool whichEye,int64_t offsetNS);
private:
    TelemetryReceiver& mTelemetryReceiver;
    MatricesManager mMatricesM;
    VRFrameTimeAccumulator mFrameTimeAcc;

    const SettingsVR mSettingsVR;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTextureExt> mGLRenderTextureExternal= nullptr;
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    std::unique_ptr<FBRManager> mFBRManager= nullptr;
    int ViewPortW=0,ViewPortH=0;
    int swapColor=0;
    void placeGLElements();
    const float MAX_FOV_USABLE_FOR_VDDC=70;

    const bool is360;
};


#endif //FPV_VR_GLRSUPERSYNC_H
