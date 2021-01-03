//
// Created by Constantin on 7/21/2018.
//

#ifndef FPV_VR_OSD_ARTIFICIALHORIZON_H
#define FPV_VR_OSD_ARTIFICIALHORIZON_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <General/IUpdateable.hpp>
#include <BasicGLPrograms.hpp>
#include <OSD/ElementBatching/BatchingManager.h>
#include <SettingsOSDStyle.h>
#include "General/IDrawable.hpp"
#include "General/IPositionable.hpp"
#include <GLBuffer.hpp>

/**
 * Artificial horizon using Ladders
 * Holds own GL Buffers
 */

class AHorizon: public IDrawable, public IPositionable, public IUpdateable{
public:
    struct Options {
        enum RENDERING_MODE{MODE_HORIZON_ONLY=0,MODE_HORIZON_WITH_3_LADDERS=1,MODE_HORIZON_WITH_5_LADDERS=2,MODE_NONE=3};
        RENDERING_MODE mode=MODE_HORIZON_WITH_3_LADDERS;
        int scale=100;
        bool roll = true, pitch = true;
        bool invRoll=false, invPitch=false;
        const bool isEnabled()const{
            return (mode==MODE_HORIZON_ONLY || mode==MODE_HORIZON_WITH_3_LADDERS || mode==MODE_HORIZON_WITH_5_LADDERS);
        };
    };
    AHorizon(const AHorizon::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver);
    Rect2D calculatePosition(const Rect2D &osdOverlay,int GLOBAL_SCALE);
private:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    // create an icon with Colored Geometry that roughly looks like this:
    //  |
    //-- --
    static ColoredMeshData createMiddleIconData(float width,float height,const TrueColor color);
    const BasicGLPrograms& mGLPrograms;
    const SettingsOSDStyle& settingsOSDStyle;
    const TelemetryReceiver& mTelemetryReceiver;
    const Options& mOptions;
    std::shared_ptr<ModifiableArray<ColoredVertex>> mMiddleTriangleBuff;
    glm::mat4 mModelMLadders;
    // How much of the video W this element uses
    const float PERCENTAGE_VIDEO_X=0.2f;
    const float RATIO=6.0f;
    struct LadderLine{
        // the value this line refers to
        int valueDegree=0;
        // begin and end of the colored vertices for this line
        std::size_t lineVertOffset=0,lineVertCount=0;
        // begin and end of the text vertices for this line
        std::size_t textVertOffset=0,textVertCount=0;
    };
    LadderLine LadderLines[1];
    float degreeToYTranslationFactor;
    std::vector<LadderLine> offsetsForLadderLines;
    //
    std::size_t currLineOffset,currLineCount;
    std::size_t currTextOffset,currTextCount;
    //
    // ladders other than the one in the middle
    GLBuffer<ColoredVertex> mGLBuffLadderLinesOther;
    GLBuffer<GLProgramText::Character> mGLBuffLadderLinesOtherText;
    float lol=0;
    ColoredGLMeshBuffer mMiddleElementBuff;
};

#endif //FPV_VR_OSD_ARTIFICIALHORIZON_H
