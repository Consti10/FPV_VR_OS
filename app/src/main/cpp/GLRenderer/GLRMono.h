/*****************************************************
 * Render OSD (optional) and Video (Optional) into a OpenGL ES Surface
 * See @enableOSD for osd options
 * See @VideoMode for video rendering modes
 ******************************************************/


#ifndef FPV_VR_GLRENDERERMONO_H
#define FPV_VR_GLRENDERERMONO_H


#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <GLProgramVC.h>
#include <GLProgramText.h>
#include <FPSCalculator.h>
#include <OSD/OSDRenderer.h>
#include "IVideoFormatChanged.hpp"
#include <VrCompositorRenderer.h>
#include <Video/VideoModesHelper.hpp>


class GLRMono: public IVideoFormatChanged{
public:
    GLRMono(JNIEnv* env, jobject androidContext, TelemetryReceiver& telemetryReceiver, gvr_context* gvr_context, VideoModesHelper::VIDEO_RENDERING_MODE videoMode, bool enableOSD);
public:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jobject optionalSurfaceTextureHolder);
    void onSurfaceChanged(int width, int height,float optionalVideo360FOV=0);
    //Draw the transparent OSD scene.
    void onDrawFrame();
    //Only in 360 degree mode
    void setHomeOrientation360();
private:
    TelemetryReceiver& mTelemetryReceiver;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const VideoModesHelper::VIDEO_RENDERING_MODE VIDEO_MODE;
    const bool ENABLE_OSD;
    const bool ENABLE_VIDEO;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    //These fields are only active when also rendering video
    struct OptionalVideoRenderer{
        std::unique_ptr<GLProgramTextureExt> glProgramTextureExt;
        TexturedGLMeshBuffer videoMesh;
        glm::mat4 monoForward360=glm::mat4(1.0f);
        glm::mat4 projectionMatrix=glm::mat4(1.0f);
        std::unique_ptr<SurfaceTextureUpdate> surfaceTextureUpdate;
    };
    OptionalVideoRenderer mOptionalVideoRender;
public:
    std::unique_ptr<gvr::GvrApi> gvr_api_;
private:
    static constexpr const float MIN_Z_DISTANCE=0.01f;
    static constexpr const float MAX_Z_DISTANCE=100.0f;

};


#endif //FPV_VR_GLRENDERERMONO_H
