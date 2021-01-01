//
// Created by Constantin on 7/21/2018.
//

#include <GLProgramText.h>
#include <SettingsOSDStyle.h>
#include <TrueColor.hpp>
#include <ColoredGeometry.hpp>
#include <GLBuffer.hpp>
#include "AHorizon.h"
#include "GLHelper.hpp"

constexpr auto TAG="AHorizon";

AHorizon::AHorizon(const AHorizon::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver &telemetryReceiver):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryReceiver(telemetryReceiver),
        mGLPrograms(basicGLPrograms),
        mPositionDebug(basicGLPrograms.vc,0, true),
        mMiddleTriangleBuff(batchingManager.allocateVCTriangles(3)),
        mOptions(options){
}

void AHorizon::addOtherLadderLine(int which,std::vector<ColoredVertex>& tmpBuffOtherLadderLines,
                                  std::vector<GLProgramText::Character>& tmpBuffOtherLadderLinesText) {

}

//
void AHorizon::setupPosition() {
    mPositionDebug.setWorldPositionDebug(mX,mY,mZ,mWidth,mHeight);
    degreeToYTranslationFactor=mHeight/180.0f;
    //make the ladder line
    {
        const float lineH=mWidth*0.015f;
        auto tmp=GLProgramLine::makeHorizontalLine({-mWidth/2.0f,0},mWidth,lineH,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        mGLBuffLadders.uploadGL(tmp);
        LadderLines[0].vertOffset=0;
        LadderLines[0].vertCount=6;

        /*const float lineH=mWidth*0.015f;
        const float lineW=mWidth;
        const float outline=lineH*0.5f;
        //std::vector<GLProgramLine::Vertex> tmp(6);
        const glm::vec3 start=glm::vec3(-lineW/2.0f-outline/2.0f,-lineH/2.0f-outline/2.0f,0);
        const glm::vec3 end=start+glm::vec3(lineW,0,0);
        //GLProgramLine::convertLineToRenderingData(start,end,lineH,tmp.data(),0,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        auto tmp=GLProgramLine::makeLine(start,end,lineH,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        mGLBuffLadders.uploadGL(tmp);
        LadderLines[0].vertOffset=0;
        LadderLines[0].vertCount=6;*/
    }
    // the other ladders
    {
        std::vector<ColoredVertex> tmpBuffOtherLadderLines;
        std::vector<GLProgramText::Character> tmpBuffOtherLadderLinesText;
        /*for(int i=1;i<5;i++){
            const float deltaBetweenLines=mHeight/180.0f*20.0f;
            const float y=(float)i*deltaBetweenLines;
            const float charHeight=mHeight/20.0f;
            const std::wstring wstring=std::to_wstring(i*20);
            const float lineWidth=mWidth-GLProgramText::getStringLength(wstring,charHeight)*2.0f;
            ColoredGeometry::addColoredLineHorizontal(tmpBuffOtherLadderLines,{-lineWidth/2.0f, y},lineWidth,TrueColor2::GREEN);
            //
            GLProgramText::appendString(tmpBuffOtherLadderLinesText,0,y-(charHeight*0.5f),0,charHeight,wstring,TrueColor2::GREEN);
        }*/
        const float charHeight=mHeight/20.0f;
        const float lineWidth=mWidth-GLProgramText::getStringLength(L"80",charHeight)*2.0f;
        const float deltaBetweenLines=mHeight/180.0f;
        int count=0;
        for(int i=-80;i<90;i+=10){
            count++;
            // skip the one in the middle (this one has an outline)
            if(i==0)continue;
            const float y=(float)i*deltaBetweenLines;
            if(count % 2 == 0){
                ColoredGeometry::addColoredLineHorizontal(tmpBuffOtherLadderLines,{-lineWidth/4.0f, y},lineWidth/2.0f,settingsOSDStyle.OSD_LINE_FILL_COLOR);
            }else{
                const std::wstring wstring=std::to_wstring(i);
                ColoredGeometry::addColoredLineHorizontal(tmpBuffOtherLadderLines,{-lineWidth/2.0f, y},lineWidth,settingsOSDStyle.OSD_LINE_FILL_COLOR);
                //
                GLProgramText::appendString(tmpBuffOtherLadderLinesText,-mWidth/2.0f,y-(charHeight*0.5f),0,charHeight,wstring,settingsOSDStyle.OSD_LINE_FILL_COLOR);
                GLProgramText::appendString(tmpBuffOtherLadderLinesText,lineWidth/2.0f,y-(charHeight*0.5f),0,charHeight,wstring,settingsOSDStyle.OSD_LINE_FILL_COLOR);
            }
        }
        mGLBuffLadderLinesOther.uploadGL(tmpBuffOtherLadderLines);
        mGLBuffLadderLinesOtherText.uploadGL(tmpBuffOtherLadderLinesText);
    }

    //make the middle triangle
    {
        float middleTriangleWidthHeight=mWidth/16.0f;
        ColoredGeometry::makeColoredTriangle1(mMiddleTriangleBuff->modify(),
                             glm::vec3(mX+mWidth/2.0f-middleTriangleWidthHeight/2.0f,mY+mHeight/2.0f-middleTriangleWidthHeight/2.0f,mZ),
                             middleTriangleWidthHeight,middleTriangleWidthHeight,
                                              TrueColor2::GREEN);
    }
    //create the 3D model
    {
        float hW=mWidth/2.0f;
        float sixtW=mWidth/6.0f;
        mGLBuff3DModel.setData(create3DModelData(hW,sixtW));
        //GLBufferHelper::uploadGLBufferStatic(mGLBuff3DModel, modelData.data(),
        //                                     modelData.size() * sizeof(GLProgramVC::Vertex));
    }
}

void AHorizon::updateGL() {
    float rollDegree= mTelemetryReceiver.getUAVTelemetryData().Roll_Deg;
    //float pitchDegree= mTelemetryReceiver.getUAVTelemetryData().Pitch_Deg;
    float pitchDegree= lol;
    lol+=0.1;
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
    glm::mat4 translM = glm::translate(glm::mat4(1.0f),glm::vec3(mX+mWidth/2.0f,mY+mHeight/2.0f,mZ));
    glm::mat4 rotateM=glm::mat4(1.0f);
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
    glm::mat4 rollRotationM=glm::rotate(glm::mat4(1.0f),glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
    const float pitchTranslY=pitchTranslationFactor*degreeToYTranslationFactor;
    glm::mat4 pitchTranslationM=glm::translate(glm::mat4(1.0f),glm::vec3(0,pitchTranslY,0));
    glm::mat4 originTranslationM = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,mZ));
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
        mGLPrograms.line.beforeDraw(mGLBuffLadders.getGLBufferId());
        mGLPrograms.line.draw(ViewM*mModelMLadders,ProjM,LadderLines[0].vertOffset,LadderLines[0].vertCount);
        mGLPrograms.line.afterDraw();
    }
    //
    glLineWidth(2.0f);
    mGLPrograms.vc.beforeDraw(mGLBuffLadderLinesOther.getGLBufferId());
    mGLPrograms.vc.draw(ViewM*mModelMLadders,ProjM,0,mGLBuffLadderLinesOther.getCount(),GL_LINES);
    mGLPrograms.vc.afterDraw();
    //
    const glm::mat4 mvp=ProjM*(ViewM*mModelMLadders);
    mGLPrograms.text.beforeDraw(mGLBuffLadderLinesOtherText.getGLBufferId());
    mGLPrograms.text.draw(mvp,0,mGLBuffLadderLinesOtherText.getCount()*6);
    mGLPrograms.text.afterDraw();
}

IPositionable::Rect2D AHorizon::calculatePosition(const IPositionable::Rect2D &osdOverlay,const int GLOBAL_SCALE) {
    float width=osdOverlay.mWidth*PERCENTAGE_VIDEO_X*(mOptions.scale/100.0f)*(GLOBAL_SCALE*0.01f);
    float height=width*RATIO;
    float x=osdOverlay.mX+osdOverlay.mWidth/2.0f-width/2.0f;//+width;
    float y=osdOverlay.mY+osdOverlay.mHeight/2.0f-height/2.0f;
    float z=osdOverlay.mZ;
    return {x,y,z,width,height};
}

ColoredMeshData AHorizon::create3DModelData(float hW, float sixtW) {
    //Copter as colored geometry data
    std::vector<ColoredVertex> vertices= {
            ColoredVertex{0.0f,0.0f,0.0f-sixtW,TrueColor2::RED}, //
            ColoredVertex{0.0f+sixtW,0.0f,0.0f+sixtW,TrueColor2::BLUE}, //bottom right
            ColoredVertex{0.0f-sixtW,0.0f,0.0f+sixtW,TrueColor2::YELLOW}, // bottom left
            //1
            ColoredVertex{0.0f,0.0f,0.0f,TrueColor2::RED},
            ColoredVertex{0.0f+hW,0.0f,0.0f-hW, TrueColor2::RED},
            ColoredVertex{0.0f+hW-(hW/4.0f),0.0f,0.0f-hW, TrueColor2::RED},
            //2
            ColoredVertex{0.0f,0.0f,0.0f, TrueColor2::BLUE},
            ColoredVertex{0.0f+hW,0.0f,0.0f+hW, TrueColor2::BLUE},
            ColoredVertex{0.0f+hW-(hW/4.0f),0.0f,0.0f+hW, TrueColor2::BLUE},
            //3
            ColoredVertex{0.0f,0.0f,0.0f, TrueColor2::YELLOW},
            ColoredVertex{0.0f-hW,0.0f,0.0f+hW, TrueColor2::YELLOW},
            ColoredVertex{0.0f-hW+(hW/4.0f),0.0f,0.0f+hW, TrueColor2::YELLOW},
            //4
            ColoredVertex{0.0f,0.0f,0.0f, TrueColor2::RED},
            ColoredVertex{0.0f-hW,0.0f,0.0f-hW, TrueColor2::RED},
            ColoredVertex{0.0f-hW+(hW/4.0f),0.0f,0.0f-hW,TrueColor2::RED}
    };
    return ColoredMeshData(vertices,GL_TRIANGLES);
}

