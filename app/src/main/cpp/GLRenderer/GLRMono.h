/*****************************************************
 * Renders OSD only onto a transparent GL Surface.
 * When playing normal video (no 360° or else) blending with video is done via Android (HW) composer.
 * This is much more efficient than rendering via a OpenGL texture
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

class GLRMono{
public:
    GLRMono(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver);
public:
    void onSurfaceCreated(JNIEnv * env,jobject obj);
    void onSurfaceChanged(int width, int height);
    //Draw the transparent OSD scene.
    void onDrawFrame(bool clearScreen);
private:
    TelemetryReceiver& mTelemetryReceiver;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
public:
    const SettingsVR mSettingsVR;
    MatricesManager mMatricesM;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
};


#endif //FPV_VR_GLRENDERERMONO_H
