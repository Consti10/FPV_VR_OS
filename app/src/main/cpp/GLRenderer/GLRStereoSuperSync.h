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
#include <GLProgramTexture.h>
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

#include "GLRStereoNormal.h"

class GLRStereoSuperSync : public GLRStereoNormal{
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
public:
    void onSurfaceCreatedX(JNIEnv * env,jobject obj,jint videoTexture);
    void onSurfaceChangedX(int width, int height);
    // we use the drawEye from GLRStereoNormal void drawEye(gvr::Eye eye);
    /**
     * Called by the SuperSync manager
     * @param env
     * @param whichEye left/right eye
     * @param offsetNS  time since eye event
     * NOTE: In SuperSync we do not use the onDrawFrame callback
     */
    void renderNewEyeCallback(JNIEnv* env,bool whichEye,int64_t offsetNS);
private:
    VRFrameTimeAccumulator mFrameTimeAcc;
    std::unique_ptr<FBRManager> mFBRManager= nullptr;
    int swapColor=0;
};


#endif //FPV_VR_GLRSUPERSYNC_H
