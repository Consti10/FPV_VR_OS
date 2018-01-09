/*****************************************************
 * Renders OSD only onto a transparent GL Surface.
 * Blending with video is done via Android (HW) composer.
 * This is much more efficient than rendering via OpenGL texture
 ******************************************************/

#ifndef FPV_VR_GLRENDERERMONO_H
#define FPV_VR_GLRENDERERMONO_H


#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>
#include <GLRenderGeometry.h>
#include <GLRenderText.h>
#include <OSDRenderer.h>
#include <Chronometer.h>
#include <FPSCalculator.h>
#include <GLRenderLine.h>

#include "gvr.h"
#include "gvr_audio.h"
#include "gvr_controller.h"
#include "gvr_types.h"
#include "IRendererEGL14.h"

class GLRendererMono{
public:
    /**
     * Create a GLRendererMono using a given |gvr_context|.
     * @param gvr_api The (non-owned) gvr_context.
     */
    GLRendererMono(gvr_context* gvr_context);

    /**
   * Destructor.
   */
    ~GLRendererMono();

    /**
     * Construct OpenGL objects.
     * @param assetManagerJAVA: a reference to the AssetManager. This is needed for the Text Atlas Texture
     * Also starts the telemetry/text elements update threads
     */
    void OnSurfaceCreated(JNIEnv * env,jobject obj,jobject assetManagerJAVA);

    /**
     * recalculate the place(s) of the OpenGL world objects
     */
    void OnSurfaceChanged(int width,int height);

    /**
   * Draw the transparent OSD scene.
   */
    void OnDrawFrame();

    /**
     * the video decoder fps will be displayed in the OSD
     */
    void setVideoDecoderFPS(float fps);

    /**
   * stop the telemetry receiver/text elements update thread(s) if existing
   */
    void OnPause();

    /**
     * Pass trough the home location lat,lon,alt. May be called multiple times until we have a high enough accuracy.
     * Is called at least once.
     */
    void setHomeLocation(double latitude, double longitude,double attitude);


private:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    std::shared_ptr<GLRenderColor> mGLRenderColor= nullptr;
    std::shared_ptr<GLRenderLine> mGLRenderLine= nullptr;
    std::shared_ptr<GLRenderText> mGLRenderText= nullptr;
    std::shared_ptr<TelemetryReceiver> mTelemetryReceiver= nullptr;
    //TODO these could be unique pointers; but c++11 has no make_unique
    std::shared_ptr<OSDRenderer> mOSDRenderer= nullptr;
    std::shared_ptr<Chronometer>CPUFrameTime=make_shared<Chronometer>("CPU FrameTime");
    std::shared_ptr<FPSCalculator>mFPSCalculator=make_shared<FPSCalculator>("OpenGL FPS",2000);

    glm::mat4x4 mViewM,mProjM;
    int64_t lastLog;

    void calculateMetrics();

};


#endif //FPV_VR_GLRENDERERMONO_H
