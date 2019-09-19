/*****************************************************
 * Renders OSD only onto a transparent GL Surface.
 * Blending with video is done via Android (HW) composer.
 * This is much more efficient than rendering via OpenGL texture
 ******************************************************/

#ifndef FPV_VR_GLRENDERERMONO360_H
#define FPV_VR_GLRENDERERMONO360_H

#include "GLRMono.h"
#include <Video/VideoRenderer.h>

class GLRMono360 : private GLRMono{
public:
    explicit GLRMono360(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,gvr_context* gvr_context,bool renderOSD);
public:
    void onSurfaceCreated360(JNIEnv * env,jobject obj,jint videoTexture);
    void onSurfaceChanged360(int width, int height);
    //Draw the 360° video, optionally also the OSD as overlay
    void onDrawFrame360();
    void setHomeOrientation();
private:
    const int renderOSD;
    std::unique_ptr<GLProgramSpherical> mGLProgramSpherical=nullptr;
    std::unique_ptr<VideoRenderer> mVideoRenderer= nullptr;
    std::unique_ptr<gvr::GvrApi> gvr_api_;
    Chronometer cpuFrameTimeVidOSD;
};


#endif //FPV_VR_GLRENDERERMONO360_H
