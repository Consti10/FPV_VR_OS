//
// Created by Constantin on 7/21/2018.
//

#ifndef FPV_VR_ARTIFICIALHORIZON_H
#define FPV_VR_ARTIFICIALHORIZON_H

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramVC.h>
#include <TelemetryReceiver.h>
#include <General/PositionDebug.hpp>
#include <General/IUpdateable.hpp>
#include <Helper/BasicGLPrograms.hpp>
#include <OSD/ElementBatching/BatchingManager.h>
#include <Settings/SettingsOSDStyle.h>
#include "General/IDrawable.hpp"
#include "General/IPositionable.hpp"
#include <GLBufferHelper.hpp>

/**
 * Artificial horizon using Ladders
 * Holds own GL Buffers
 */

class AHorizon: public IDrawable, public IPositionable, public IUpdateable{
public:
    struct Options {
        int mode=MODE_BOTH_TOGETHER;
        int scale=100;
        bool roll = true, pitch = true;
        bool invRoll=false, invPitch=false;
        const bool isEnabled()const{
            return (mode==MODE_3D_QUADCOPTER || mode==MODE_2D_LADDERS || mode==MODE_BOTH_TOGETHER );
        };
    };
    AHorizon(const AHorizon::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver);
    Rect2D calculatePosition(const Rect2D &osdOverlay,int GLOBAL_SCALE);
    static constexpr int MODE_3D_QUADCOPTER = 0;
    static constexpr int MODE_2D_LADDERS = 1;
    static constexpr int MODE_BOTH_TOGETHER = 2;
    static constexpr int MODE_NONE=3;
private:
    void setupPosition() override;
    void updateGL() override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) override;
    std::array<GLProgramVC::Vertex,3+4*3> create3DModelData(float hw,float sixtW);
    const BasicGLPrograms& mGLPrograms;
    const SettingsOSDStyle& settingsOSDStyle;
    const TelemetryReceiver& mTelemetryReceiver;
    const Options& mOptions;
    PositionDebug mPositionDebug;
    VertexBuffer mGLBuffLadders;
    ModifiableArray<GLProgramVC::Vertex>* mMiddleTriangleBuff;
    glm::mat4x4 mModelMLadders;
    const float PERCENTAGE_VIDEO_X=0.2f;
    const float RATIO=1.5f;
    struct LadderLine{
        int vertOffset=0,vertCount=0;
    };
    LadderLine LadderLines[1];
    float degreeToYTranslationFactor;
    //
    VertexBuffer mGLBuff3DModel;
    glm::mat4x4 mModelM3DModel;
};

#endif //FPV_VR_ARTIFICIALHORIZON_H
