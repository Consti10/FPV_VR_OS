
#ifndef OSDRENDERER
#define OSDRENDERER

#include <GLES2/gl2.h>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <Helper/BasicGLPrograms.hpp>
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
#include <GLBufferHelper.hpp>

/**
 * This class exists to make the GLRendererXXX much more slim, since positioning all the OSD elements
 * at the right place needs quite a lot of code.
 */

class OSDRenderer{
public:
    OSDRenderer(JNIEnv* env,jobject androidContext,const BasicGLPrograms& basicGLPrograms,TelemetryReceiver& telemetryReceiver);
    void placeGLElementsMono(const IPositionable::Rect2D& rectViewport);
    void placeGLElementsStereo(const IPositionable::Rect2D& rectVideoCanvas);
    void updateAndDrawElementsGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    void drawElementsGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    const SettingsOSDStyle settingsOSDStyle;
    const SettingsOSDElements settingsOSDElements;
private:
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
};

#endif

