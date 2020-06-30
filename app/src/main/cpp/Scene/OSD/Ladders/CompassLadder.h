
#ifndef OSD_COMPASLADDER
#define OSD_COMPASLADDER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <BasicGLPrograms.hpp>
#include <General/IUpdateable.hpp>
#include <General/ITextHeight.h>
#include <OSD/ElementBatching/BatchingManager.h>
#include <OSD/ATextElements/OSDTextObj.hpp>
#include <OSD/ATextElements/OSDBackgroundObj.hpp>
#include <SettingsOSDStyle.h>
#include <GLBuffer.hpp>
#include "../../General/IDrawable.hpp"
#include "../../General/IPositionable.hpp"
#include "../../General/PositionDebug.hpp"

/**
 * Draw an OSD compass ladder
 * Holds its own gl Buffers (for abstraction,so the performance loss is justifiable)
 * But as of 10.8.2018, the CurrentHeadingElement (with outlines) uses the DynamicTextManager
 */

class CompassLadder : public IDrawable,public IPositionable,public IUpdateable{
public:
    struct Options{
        bool enable = true;
        int scale=100;
        bool homeArrow = true;
        bool invHeading=false;
        bool COGoverMag=false;
    };
    CompassLadder(CompassLadder::Options options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver);
    static constexpr float PERCENTAGE_VIDEO_Y_MONO=3.0f/20.0f;
    static constexpr float PERCENTAGE_VIDEO_Y_STEREO=2.4f/20.0f;
    static constexpr float RATIO=4.0f/1.0f;
    static constexpr float TEXT_UPSCALE=1.3f;
    Rect2D calculatePosition(const Rect2D &osdOverlay, bool stereo);
private:
    void setupPosition() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    void updateGL() override ;
    const BasicGLPrograms& mGLPrograms;
    const SettingsOSDStyle& settingsOSDStyle;
    const Options mOptions;
    const TelemetryReceiver& mTelemetryReceiver;
    OSDTextObj mTextObjTelemetryValue;
    OSDBackgroundObject mBackgroundObj;
    PositionDebug mPositionDebug;
    GLBuffer<GLProgramLine::Vertex> mGLLadderLinesB;
    GLBuffer<GLProgramText::Character> mGLLadderTextB;
    GLBuffer<GLProgramText::Character> mGLHomeIconB;
    ModifiableArray<GLProgramVC::Vertex>* mMiddleArrow;
    glm::mat4 mHeadingTranslM;
    glm::mat4 mHomeArrowTM;
    float degreeInGLTranslation=0;
    float mCalcTextHeight;
    int charOffset=0,nCharsToDraw=0;
    int linesOffset,nLinesToDraw=0;
    static constexpr int N_EXISTING_LADDER_LINES=4*4*2; //4*4*2 Lines for the S|||| from compas
    static constexpr int N_EXISTING_CHARS=4*2;//8chars for N,W,S,E*2
    static constexpr const int N_CHARS_PER_TEXT_OBJ=15;
};

#endif

