/*****************************************************
 * Renders Video and OSD side by side with OpenGL
 ******************************************************/

#ifndef FPV_VR_GLRSUPERSYNC_H
#define FPV_VR_GLRSUPERSYNC_H

#include "jni.h"
#include "OSDRenderer.h"
#include "VideoRenderer.h"
#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderTextureExternal.h>
#include <TelemetryReceiver.h>
#include <EGL/egl.h>
#include <inttypes.h>
#include "../SettingsN.h"
#include "../Helper/Extensions.h"
#include "../Helper/CPUPriorities.h"
#include <sys/resource.h>
#include <FBRManager.h>
#include <FrameCPUChronometer.h>
#include <gvr_types.h>
#include <HeadTrackerExtended.h>

class GLRSuperSync {
public:
    /**
     * Create a GLRenderer Stereo SuperSync using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    GLRSuperSync(gvr_context* gvr_context);

    /**
   * Destructor.
   */
    ~GLRSuperSync();

    /**
     * Construct OpenGL world objects.
     * @param assetManagerJAVA: a reference to the AssetManager. This is needed for the Text Atlas Texture
     * Also starts the telemetry/text elements update threads
     */
    void OnSurfaceCreated(JNIEnv * env,jobject obj,jint videoTexture, jobject assetManagerJAVA,bool qcomTiledRenderingAvailable);

    /**
     * recalculate the place(s) of the OpenGL world objects
     */
    void OnSurfaceChanged(int width,int height);

    /**
    * Draw the Video and transparent OSD scene, synchronized with the VSYNC, directly into the Front Buffer
     * This is called in OnDrawFrame and does not return until exitSuperSyncLoop is called.
    */
    void enterSuperSyncLoop(JNIEnv * env, jobject obj,jobject surfaceTexture,int exclusiveVRCore);

    /**
     * Exit the SuperSync loop. Since the super sync loop blocks the GLThread, this has to be called from another thread, e.g the UI thread
     * called by the UI's onPause().
     */
    void exitSuperSyncLoop();

    /**
    * the video decoder fps will be displayed in the OSD
    */
    void setVideoDecoderFPS(float fps);


    /**
     * The OpenGL renderer might start before the first video frame is available (inititalized by default with 1280x720)
     * If the video ratio changes, a recalculation of the scene object positions is required
     * @param videoW Video width, in pixels
     * @param videoH Video height, in pixels
     * The recalculation happens in OnDrawFrame, since it needs a valid OpenGL context
     */
    void OnVideoRatioChanged(int videoW,int videoH);

    /**
     * Pass trough the last VSYNC from jav
     * @param lastVSYNC last reported vsync, in ns
     */
    void doFrame(int64_t lastVSYNC);

    /**
    * stop the telemetry receiver/text elements update thread(s) if existing
    */
    void OnGLSurfaceDestroyed();

    /**
     * Called by the SuperSync manager
     * @param env
     * @param whichEye left/right eye
     * @param offsetNS  time since eye event
     */
    void renderNewEyeCallback(JNIEnv* env,bool whichEye,int64_t offsetNS);

private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::shared_ptr<GLRenderColor> mGLRenderColor= nullptr;
    std::shared_ptr<GLRenderText> mGLRenderText= nullptr;
    std::shared_ptr<GLRenderTextureExternal> mGLRenderTextureExternal= nullptr;
    std::shared_ptr<VideoRenderer> mVideoRenderer= nullptr;
    std::shared_ptr<TelemetryReceiver> mTelemetryReceiver= nullptr;
    //TODO these could be unique pointers; but c++11 has no make_unique
    std::shared_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::shared_ptr<FBRManager> mFBRManager= nullptr;
    std::shared_ptr<FrameCPUChronometer> mFrameCPUChronometer= nullptr;
    std::shared_ptr<HeadTrackerExtended>mHeadTrackerExtended= nullptr;

    void drawEye(bool whichEye);
    int ViewPortW=0,ViewPortH=0;
    int WindowW=0,WindowH=0;

    bool videoFormatChanged;
    float videoFormat=1.77777f;

    const float MAX_Z_DISTANCE=14.0f;

    int color=0;
    bool changeSwapColor= false;
    void placeGLElements();
};


#endif //FPV_VR_GLRSUPERSYNC_H
