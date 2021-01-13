
#ifndef OSDRENDERER
#define OSDRENDERER

#include <GLES2/gl2.h>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <BasicGLPrograms.hpp>
#include <OSD/Ladders/CompassLadder.h>
#include <OSD/Text/TextElements2.h>
#include <OSD/Text/TextElements1.h>
#include <OSD/ArtificialHorizon/AHorizon.h>
#include <OSD/Text/TEWarning.h>
#include <OSD/Ladders/AVerticalLadder.h>
#include <OSD/Ladders/VLAltitude.h>
#include <OSD/Ladders/VLSpeed.h>
#include <OSD/ElementBatching/BatchingManager.h>
#include <SettingsOSDElements.h>
#include <SettingsOSDStyle.h>
#include <GLBuffer.hpp>

/**
 * This class exists to make the GLRendererXXX much more slim, since positioning all the OSD elements
 * at the right place needs quite a lot of code.
 */

class OSDRenderer{
public:
    OSDRenderer(JNIEnv* env,jobject androidContext,TelemetryReceiver& telemetryReceiver,bool stereo,int WIDTH_PX,int HEIGHT_PX);
private:
    void placeGLElements();
public:
    void updateAndDrawElementsGL();
    const SettingsOSDStyle settingsOSDStyle;
    const SettingsOSDElements settingsOSDElements;
    //
    static constexpr const float MIN_Z_DISTANCE=0.01f;
    static constexpr const float MAX_Z_DISTANCE=100.0f;
    const glm::mat4 IDENTITY_M=glm::mat4(1.0f);
    // orthographic,setup in place()
    glm::mat4 mOSDProjectionM{};
private:
    const bool stereo;
    BasicGLPrograms mBasicGLPrograms;
    BatchingManager mBatchingManager;
    TelemetryReceiver& mTelemetryReceiver;
    //OSD elements
    AHorizon* mAHorizon=nullptr;
    CompassLadder* mCompassLadder= nullptr;
    VLAltitude* mAltitudeLadder= nullptr;
    VLSpeed* mSpeedLadder= nullptr;
    TextElements1* mTextElements1= nullptr;
    TextElements2* mTextElements2=nullptr;
    TEWarning* mTEWarning=nullptr;
    std::vector<IDrawable*> mDrawables={};
    std::vector<IUpdateable*> mUpdateables={};
    std::chrono::steady_clock::time_point mFLightStart;
    //
    const int WIDTH_PX,HEIGHT_PX;
};

#endif

