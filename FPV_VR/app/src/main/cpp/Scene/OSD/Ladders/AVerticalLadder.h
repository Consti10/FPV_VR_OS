//
// Created by Constantin on 8/17/2018.
//

#ifndef FPV_VR_VERTICALLADDER_H
#define FPV_VR_VERTICALLADDER_H

#include <glm/mat4x4.hpp>
#include <GLES2/gl2.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramText.h>
#include <GLProgramVC.h>
#include <General/IDrawable.hpp>
#include <General/IPositionable.hpp>
#include <TelemetryReceiver.h>
#include <General/PositionDebug.hpp>
#include <General/IUpdateable.hpp>
#include <OSD/ElementBatching/BatchingManager.h>
#include <OSD/ATextElements/OSDTextObj.hpp>
#include <OSD/ATextElements/OSDBackgroundObj.hpp>
#include <Settings/SettingsOSDStyle.h>

class AVerticalLadder : public IDrawable, public IPositionable, public IUpdateable  {
public:
    AVerticalLadder(const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver,
                   bool leftHanded, int unitsBetween,const std::wstring& unit);
    static constexpr float PERCENTAGE_VIDEO_Y_MONO=10.0f/20.0f;
    static constexpr float PERCENTAGE_VIDEO_Y_STEREO=8.0f/20.0f;
    static constexpr float RATIO=1.0f/2.5f;
protected:
    float verticalLadderValue=0;
    void updateGL() override;
    const TelemetryReceiver& mTelemetryReceiver;
    const SettingsOSDStyle& settingsOSDStyle;
private:
    void setupPosition() override ;
    void updateLadderStringsRange(int middleValue);
    void updateMainString(float value);
    void calcLadderLinesRenderData(float value);
    void calcLadderStringsRenderData(float value);
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    const BasicGLPrograms& mGLPrograms;
    PositionDebug mPositionDebug;
    OSDBackgroundObject mBackgroundObj;
    OSDTextObj mTextObjTelemetryValue;

    const std::wstring UNIT;
    const bool LEFT_HANDED;
    const int N_LADDER_LINES=5*4;
    const int UNITS_BETWEEN_LONG_LINES;
    const int UNITS_BETWEEN_SRINGS;
    const int PRECALCULATED_RANGE_BOOTH_SIDES=1000;
    const int N_LADDER_STRINGS=(PRECALCULATED_RANGE_BOOTH_SIDES*2)/UNITS_BETWEEN_SRINGS;
    const int MAX_N_CHARS_PER_LADDER_STRING=6+(int)UNIT.length(); //-99999, -1.23

    float outlineQuadWidth;
    float outlineQuadHeight;
    struct LadderLines{
        int currentDrawOffset;
        int currentDrawNumber;
        GLuint glBuffer;
    }mLadderLines;
    glm::mat4x4 mLadderLinesTM;

    struct LadderStrings{
        int currentDrawOffset1;
        int currentDrawNumber1;
        int currentDrawOffset2;
        int currentDrawNumber2;
        GLuint glBuffer;
        int currentMiddleValue;
        glm::mat4x4 currTranslationM;
    }mLadderStrings;
};

//0 and 1000: 0
//990 and 1000: 1000
//500 and 1000: 1000
//-500 and 1000: -1000
static int roundToMultiple(int numToRound,int multiple){
    int remainder=abs(numToRound)%multiple;
    if(remainder==0)return numToRound;
    int ret;
    if(remainder<multiple/2)
        ret=abs(numToRound)-remainder;
    else
        ret=abs(numToRound)+multiple-remainder;
    if(numToRound<0){
        return -ret;
    }else{
        return ret;
    }
}

#endif //FPV_VR_VERTICALLADDER_H
