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
#include <MatricesManager.h>
#include <OSD/OSDRenderer.h>
#include <IVideoFormatChanged.hpp>
#include <Chronometer.h>
#include <Video/VideoRenderer.h>

class GLRMono{
public:
    enum VIDEO_MODE{NONE,STEREO,Degree360};
public:
    GLRMono(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,VIDEO_MODE videoMode,bool enableOSD);
public:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint optionalVideoTexture=0);
    void onSurfaceChanged(int width, int height,float optionalVideo360FOV=0);
    //Draw the transparent OSD scene.
    void onDrawFrame();
    //Only in 360 degree mode
    void setHomeOrientation360();
private:
    TelemetryReceiver& mTelemetryReceiver;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const VIDEO_MODE videoMode;
    const bool enableOSD;
public:
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<GLProgramTexture> mGLProgramTexture=nullptr;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
    //These fields are only active when also rendering video
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
private:
    const glm::mat4 mViewM=glm::mat4(1.0f);
    glm::mat4 mOSDProjectionM=glm::mat4(1.0f);
    glm::mat4 m360ProjectionM=glm::mat4(1.0f);
    glm::mat4 monoForward360=glm::mat4(1.0f);
    static constexpr const float MIN_Z_DISTANCE=0.01f;
    static constexpr const float MAX_Z_DISTANCE=100.0f;
};


#endif //FPV_VR_GLRENDERERMONO_H
