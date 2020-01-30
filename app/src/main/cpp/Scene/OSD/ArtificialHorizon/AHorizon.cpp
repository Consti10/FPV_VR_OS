//
// Created by Constantin on 7/21/2018.
//

#include <GLProgramText.h>
#include <SettingsOSDStyle.h>
#include <Color/Color.hpp>
#include <GeometryBuilder/ColoredGeometry.hpp>
#include <Helper/GLBufferHelper.hpp>
#include "AHorizon.h"
#include "Helper/GLHelper.hpp"

#define TAG "AHorizon"
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

AHorizon::AHorizon(const AHorizon::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver &telemetryReceiver):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryReceiver(telemetryReceiver),
        mGLPrograms(basicGLPrograms),
        mPositionDebug(basicGLPrograms.vc,0, true),
        mMiddleTriangleBuff(batchingManager.allocateVCTriangles(3)),
        mOptions(options){
    mGLBuffLadders.initializeGL();
    mGLBuff3DModel.initializeGL();
}

//
void AHorizon::setupPosition() {
    mPositionDebug.setWorldPositionDebug(mX,mY,mZ,mWidth,mHeight);
    degreeToYTranslationFactor=mHeight/180.0f;
    //make the ladder line
    {
        const float lineH=mWidth*0.015f;
        const float lineW=mWidth;
        const float outline=lineH*0.5f;
        std::vector<GLProgramLine::Vertex> tmp(6);
        const glm::vec3 start=glm::vec3(-lineW/2.0f-outline/2.0f,-lineH/2.0f-outline/2.0f,0);
        const glm::vec3 end=start+glm::vec3(lineW,0,0);
        GLProgramLine::convertLineToRenderingData(start,end,lineH,tmp.data(),0,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        mGLBuffLadders.uploadGL(tmp);

        LadderLines[0].vertOffset=0;
        LadderLines[0].vertCount=6;
    }
    //make the middle triangle
    {
        float middleTriangleWidthHeight=mWidth/16.0f;
        ColoredGeometry::makeColoredTriangle1(mMiddleTriangleBuff->modify(),
                             glm::vec3(mX+mWidth/2.0f-middleTriangleWidthHeight/2.0f,mY+mHeight/2.0f-middleTriangleWidthHeight/2.0f,mZ),
                             middleTriangleWidthHeight,middleTriangleWidthHeight,
                                              Color::GREEN);
    }
    //create the 3D model
    {
        float hW=mWidth/2.0f;
        float sixtW=mWidth/6.0f;
        auto modelData=create3DModelData(hW,sixtW);
        mGLBuff3DModel.uploadGL(modelData);
        //GLBufferHelper::uploadGLBufferStatic(mGLBuff3DModel, modelData.data(),
        //                                     modelData.size() * sizeof(GLProgramVC::Vertex));
    }
}

void AHorizon::updateGL() {
    float rollDegree= mTelemetryReceiver.getUAVTelemetryData().Roll_Deg;
    float pitchDegree= mTelemetryReceiver.getUAVTelemetryData().Pitch_Deg;
    if(!mOptions.roll){
        rollDegree=0.0f;
    }
    if(mOptions.invRoll){
        rollDegree*=-1.0f;
    }
    if(!mOptions.pitch){
        pitchDegree=0.0f;
    }
    if(mOptions.invPitch){
        pitchDegree*=-1.0f;
    }
    //for the 3d Model:
    glm::mat4x4 translM = glm::translate(glm::mat4(1.0f),glm::vec3(mX+mWidth/2.0f,mY+mHeight/2.0f,mZ));
    glm::mat4x4 rotateM=glm::mat4(1.0f);
    rotateM=glm::rotate(rotateM,glm::radians(pitchDegree), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateM=glm::rotate(rotateM,glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
    mModelM3DModel=translM*rotateM;

    //we need the pitchDegree expressed in the range from -90 to 90 degrees.
    //where +90 stays +90 and +91 becomes -89
    //first bring it into the range 0...360
    pitchDegree=std::fmod(pitchDegree,360.0f);
    if(pitchDegree<0){
        pitchDegree+=360;
    }
    //now pitchDegree is in the range of 0...360
    float pitchTranslationFactor=pitchDegree;
    //for the ladders, a rotation of 180° is the same as 0°
    pitchTranslationFactor=std::fmod(pitchTranslationFactor,180.0f);
    if(pitchTranslationFactor>90){
        pitchTranslationFactor-=180;
    }
    glm::mat4x4 rollRotationM=glm::rotate(glm::mat4(1.0f),glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
    const float pitchTranslY=pitchTranslationFactor*degreeToYTranslationFactor;
    glm::mat4x4 pitchTranslationM=glm::translate(glm::mat4(1.0f),glm::vec3(0,pitchTranslY,0));
    glm::mat4x4 originTranslationM = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,mZ));
    mModelMLadders=rollRotationM*(originTranslationM*pitchTranslationM);
}

void AHorizon::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    mPositionDebug.drawGLDebug(ViewM,ProjM);

    //middle triangle is rendered by batching manager

    //Render the 3D Quadcopter representation
    if(mOptions.mode==MODE_3D_QUADCOPTER
       || mOptions.mode==MODE_BOTH_TOGETHER){
        mGLPrograms.vc.drawX(ViewM*mModelM3DModel,ProjM,mGLBuff3DModel);
    }

    //Render the lines
    if(mOptions.mode==MODE_2D_LADDERS
       || mOptions.mode==MODE_BOTH_TOGETHER){
        mGLPrograms.line.beforeDraw(mGLBuffLadders.vertexB);
        mGLPrograms.line.draw(ViewM*mModelMLadders,ProjM,LadderLines[0].vertOffset,LadderLines[0].vertCount);
        mGLPrograms.line.afterDraw();
    }

}

IPositionable::Rect2D AHorizon::calculatePosition(const IPositionable::Rect2D &osdOverlay,const int GLOBAL_SCALE) {
    float width=osdOverlay.mWidth*PERCENTAGE_VIDEO_X*(mOptions.scale/100.0f)*(GLOBAL_SCALE*0.01f);
    float height=width*RATIO;
    float x=osdOverlay.mX+osdOverlay.mWidth/2.0f-width/2.0f;//+width;
    float y=osdOverlay.mY+osdOverlay.mHeight/2.0f-height/2.0f;
    float z=osdOverlay.mZ;
    return {x,y,z,width,height};
}

std::array<GLProgramVC::Vertex,3+4*3> AHorizon::create3DModelData(float hW, float sixtW) {
    //Copter as colored geometry data
    return {
            GLProgramVC::Vertex{0.0f,0.0f,0.0f-sixtW,Color::RED}, //
            GLProgramVC::Vertex{0.0f+sixtW,0.0f,0.0f+sixtW,Color::BLUE}, //bottom right
            GLProgramVC::Vertex{0.0f-sixtW,0.0f,0.0f+sixtW,Color::YELLOW}, // bottom left
            //1
            GLProgramVC::Vertex{0.0f,0.0f,0.0f,Color::RED},
            GLProgramVC::Vertex{0.0f+hW,0.0f,0.0f-hW, Color::RED},
            GLProgramVC::Vertex{0.0f+hW-(hW/4.0f),0.0f,0.0f-hW, Color::RED},
            //2
            GLProgramVC::Vertex{0.0f,0.0f,0.0f, Color::BLUE},
            GLProgramVC::Vertex{0.0f+hW,0.0f,0.0f+hW, Color::BLUE},
            GLProgramVC::Vertex{0.0f+hW-(hW/4.0f),0.0f,0.0f+hW, Color::BLUE},
            //3
            GLProgramVC::Vertex{0.0f,0.0f,0.0f, Color::YELLOW},
            GLProgramVC::Vertex{0.0f-hW,0.0f,0.0f+hW, Color::YELLOW},
            GLProgramVC::Vertex{0.0f-hW+(hW/4.0f),0.0f,0.0f+hW, Color::YELLOW},
            //4
            GLProgramVC::Vertex{0.0f,0.0f,0.0f, Color::RED},
            GLProgramVC::Vertex{0.0f-hW,0.0f,0.0f-hW, Color::RED},
            GLProgramVC::Vertex{0.0f-hW+(hW/4.0f),0.0f,0.0f-hW,Color::RED}
    };
}
