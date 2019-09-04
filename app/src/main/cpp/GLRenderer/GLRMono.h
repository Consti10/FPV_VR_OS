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
#include <thread>
#include <vector>
#include <GLProgramVC.h>
#include <GLProgramText.h>
#include <FPSCalculator.h>
#include <MatricesManager.h>
#include <OSD/OSDRenderer.h>
#include <IVideoFormatChanged.hpp>
#include <Chronometer.h>

#include "IGLRenderer.h"


class GLRMono : public IGLRenderer,public IVideoFormatChanged{
public:
    explicit GLRMono(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver);
private:
    void onSurfaceCreated(JNIEnv * env,jobject obj,jint optionalVideoTexture) override;
    void onSurfaceChanged(int width, int height)override;
    //Draw the transparent OSD scene.
    void onDrawFrame()override ;
private:
    TelemetryReceiver& mTelemetryReceiver;
    MatricesManager mMatricesM;
    Chronometer cpuFrameTime;
    FPSCalculator mFPSCalculator;
    const SettingsVR mSettingsVR;
    std::unique_ptr<BasicGLPrograms> mBasicGLPrograms=nullptr;
    std::unique_ptr<OSDRenderer> mOSDRenderer= nullptr;
};


#endif //FPV_VR_GLRENDERERMONO_H
