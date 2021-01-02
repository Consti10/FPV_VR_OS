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
        mMiddleTriangleBuff(batchingManager.allocateVCTriangles(3)),
        mOptions(options){
}

// adds a dashed line starting at @param point1 having a total length of @param w
static void addColoredLineHorizontalDashed(std::vector<ColoredVertex>& buff, const glm::vec2& point1, const float w,const TrueColor color1){
    float dashWidth=w/5.0f;
    for(int i=0;i<5;i++){
        if(i % 2 !=0)continue;
        glm::vec2 start=point1+glm::vec2(i*dashWidth,0);
        ColoredGeometry::addColoredLineHorizontal(buff,start,dashWidth,color1);
    }
}

// adds colored geometry for exactly one OSD ladder line
// it is centered around (0,@param y) and has a width of @param w, leaving a gap in the middle of @param spaceToLeaveFreeInTheMiddle
static void addColoredLineHorizontalCustom(std::vector<ColoredVertex>& buff, const float y, const float w,const TrueColor color1,const float spaceToLeaveFreeInTheMiddle,const bool useDashes){
    const glm::vec2 beginLeft=glm::vec2(-w/2.0f,y);
    const glm::vec2 beginRight=glm::vec2(spaceToLeaveFreeInTheMiddle/2.0f,y);
    const float lineWidthLeftOrRight=(w-spaceToLeaveFreeInTheMiddle)*0.5f;
    if(useDashes){
        addColoredLineHorizontalDashed(buff,beginLeft,lineWidthLeftOrRight,color1);
        addColoredLineHorizontalDashed(buff,beginRight,lineWidthLeftOrRight,color1);
    }else{
        ColoredGeometry::addColoredLineHorizontal(buff,beginLeft,lineWidthLeftOrRight,color1);
        ColoredGeometry::addColoredLineHorizontal(buff,beginRight,lineWidthLeftOrRight,color1);
    }
}


//
void AHorizon::setupPosition() {
    degreeToYTranslationFactor=mHeight/180.0f;
    //make the ladder line
    /*{
        const float lineH=mWidth*0.015f;
        auto tmp=GLProgramLine::makeHorizontalLine({-mWidth/2.0f,0},mWidth,lineH,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        mGLBuffLadders.uploadGL(tmp);
        LadderLines[0].vertOffset=0;
        LadderLines[0].vertCount=6;
    }*/
    const float spaceInTheMiddle=mWidth*0.2f;
    // make the "middle element" that doesn't move
    {
        mMiddleElementBuff.setData(createMiddleIconData(spaceInTheMiddle,spaceInTheMiddle*0.2f,settingsOSDStyle.OSD_LINE_FILL_COLOR));
    }
    // the other ladders
    {
        std::vector<ColoredVertex> tmpBuffOtherLadderLines;
        std::vector<GLProgramText::Character> tmpBuffOtherLadderLinesText;
        const float charHeight=mHeight/60.0f;
        const float lineWidth=mWidth-GLProgramText::getStringLength(L"80",charHeight)*2.0f;
        const float deltaBetweenLines=mHeight/180.0f;
        int count=0;
        for(int i=-180;i<=180;i+=10){
            count++;
            const float y=(float)i*deltaBetweenLines;
            // the one at 0° has no text and is "bigger"
            if(i==0){
                const auto offsetBefore=tmpBuffOtherLadderLines.size();
                addColoredLineHorizontalCustom(tmpBuffOtherLadderLines,y,mWidth,settingsOSDStyle.OSD_LINE_FILL_COLOR,spaceInTheMiddle,false);
                offsetsForLadderLines.push_back({i,offsetBefore,tmpBuffOtherLadderLines.size()-offsetBefore,tmpBuffOtherLadderLinesText.size(),0});
                continue;
            }
            const auto offsetBeforeLines=tmpBuffOtherLadderLines.size();
            const auto offsetBeforeText=tmpBuffOtherLadderLinesText.size();

            // make a longer line with text on the side
            const std::wstring text=std::to_wstring(std::abs(i));
            const float textLength=GLProgramText::getStringLength(text, charHeight);
            const bool useDashes=i<0;
            // in the negative range the lines are dashed
            addColoredLineHorizontalCustom(tmpBuffOtherLadderLines,y,lineWidth,settingsOSDStyle.OSD_LINE_FILL_COLOR,spaceInTheMiddle,useDashes);
            // if we are in the positive / negative range also add an indicator for up/down
            // this adds the vertical dashes on the left/right side pointing up/down for negative/positive values, respectively
            if(i>0){
                ColoredGeometry::addColoredLineVertical(tmpBuffOtherLadderLines,{-lineWidth*0.5f, y},-charHeight*0.5f,settingsOSDStyle.OSD_LINE_FILL_COLOR);
                ColoredGeometry::addColoredLineVertical(tmpBuffOtherLadderLines,{lineWidth*0.5f, y},-charHeight*0.5f,settingsOSDStyle.OSD_LINE_FILL_COLOR);
            }else if(i<0){
                ColoredGeometry::addColoredLineVertical(tmpBuffOtherLadderLines,{-lineWidth*0.5f, y},charHeight*0.5f,settingsOSDStyle.OSD_LINE_FILL_COLOR);
                ColoredGeometry::addColoredLineVertical(tmpBuffOtherLadderLines,{lineWidth*0.5f, y},charHeight*0.5f,settingsOSDStyle.OSD_LINE_FILL_COLOR);
            }
            auto textColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
            if(i<0){
                textColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR1;
            }
            //
            GLProgramText::appendString(tmpBuffOtherLadderLinesText,-mWidth/2.0f,y-(charHeight*0.5f), 0, charHeight, text, textColor);
            GLProgramText::appendString(tmpBuffOtherLadderLinesText,(mWidth*0.5f)-textLength,y-(charHeight*0.5f), 0, charHeight, text, textColor);
            offsetsForLadderLines.push_back({i,offsetBeforeLines,tmpBuffOtherLadderLines.size()-offsetBeforeLines,offsetBeforeText,tmpBuffOtherLadderLinesText.size()-offsetBeforeText});
        }
        mGLBuffLadderLinesOther.uploadGL(tmpBuffOtherLadderLines);
        mGLBuffLadderLinesOtherText.uploadGL(tmpBuffOtherLadderLinesText);
        assert(offsetsForLadderLines.size()==count);
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
    //float pitchDegree= lol;
    float pitchDegree=10.0f;
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
    {
        //for the 3d Model:
        glm::mat4 rotateM=glm::mat4(1.0f);
        rotateM=glm::rotate(rotateM,glm::radians(pitchDegree), glm::vec3(1.0f, 0.0f, 0.0f));
        rotateM=glm::rotate(rotateM,glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
        mModelM3DModel=rotateM;
    }

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
    if(pitchTranslationFactor>90) {
        pitchTranslationFactor -= 180;
    }
    glm::mat4 rollRotationM=glm::rotate(glm::mat4(1.0f),glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
    const float pitchTranslY=pitchTranslationFactor*degreeToYTranslationFactor;
    glm::mat4 pitchTranslationM=glm::translate(glm::mat4(1.0f),glm::vec3(0,pitchTranslY,0));
    mModelMLadders=rollRotationM*(pitchTranslationM);
    //
    for(int i=0;i<offsetsForLadderLines.size();i++){
        if(offsetsForLadderLines[i].valueDegree==-(int)pitchTranslationFactor){
            currLadderLineClosestToTheMiddle=&offsetsForLadderLines[i];
            break;
        }
    }
    assert(currLadderLineClosestToTheMiddle!= nullptr);
    //currLadderLineClosestToTheMiddle=&offsetsForLadderLines.at((int)pi)
    currLineOffset=currLadderLineClosestToTheMiddle->lineVertOffset;
    currLineCount=currLadderLineClosestToTheMiddle->lineVertCount;
}

void AHorizon::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    //debug(mGLPrograms.vc,ViewM,ProjM);

    //middle triangle is rendered by batching manager

    //Render the 3D Quadcopter representation
    if(mOptions.mode==MODE_3D_QUADCOPTER
       || mOptions.mode==MODE_BOTH_TOGETHER){
        mGLPrograms.vc.drawX(ViewM*mModelM3DModel,ProjM,mGLBuff3DModel);
    }

    //Render the lines
    /*if(mOptions.mode==MODE_2D_LADDERS
       || mOptions.mode==MODE_BOTH_TOGETHER){
        mGLPrograms.line.beforeDraw(mGLBuffLadders.getGLBufferId());
        //mGLPrograms.line.draw(ViewM*mModelMLadders,ProjM,LadderLines[0].vertOffset,LadderLines[0].vertCount);
        mGLPrograms.line.afterDraw();
    }*/

    // draw the lines for "other lines"
    glLineWidth(2.0f);

    /*mGLPrograms.vc.beforeDraw(mGLBuffLadderLinesOther.getGLBufferId());
    mGLPrograms.vc.draw(ViewM*mModelMLadders,ProjM,0,mGLBuffLadderLinesOther.getCount(),GL_LINES);
    mGLPrograms.vc.afterDraw();*/
    mGLPrograms.vc.beforeDraw(mGLBuffLadderLinesOther.getGLBufferId());
    mGLPrograms.vc.draw(ViewM*mModelMLadders,ProjM,currLineOffset,currLineCount,GL_LINES);
    mGLPrograms.vc.afterDraw();

    // draw the text for "other lines"
    const glm::mat4 mvp=ProjM*(ViewM*mModelMLadders);
    mGLPrograms.text.beforeDraw(mGLBuffLadderLinesOtherText.getGLBufferId());
    mGLPrograms.text.draw(mvp,0,mGLBuffLadderLinesOtherText.getCount()*6);
    mGLPrograms.text.afterDraw();

    mGLPrograms.vc.drawX(ViewM,ProjM,mMiddleElementBuff);
}

IPositionable::Rect2D AHorizon::calculatePosition(const IPositionable::Rect2D &osdOverlay,const int GLOBAL_SCALE) {
    float width=osdOverlay.mWidth*PERCENTAGE_VIDEO_X*(mOptions.scale/100.0f)*(GLOBAL_SCALE*0.01f);
    float height=width*RATIO;
    float x=osdOverlay.mX+osdOverlay.mWidth/2.0f-width/2.0f;//+width;
    float y=osdOverlay.mY+osdOverlay.mHeight/2.0f-height/2.0f;

    return {x,y,width,height};
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

ColoredMeshData AHorizon::createMiddleIconData(float width,float height,const TrueColor color) {
    std::vector<ColoredVertex> tmp;
    tmp.reserve(64);
    const float height1=height*0.5f;
    const float width1=width-height1;
    auto lineHorizontal1=ColoredGeometry::makeColoredRectangle({-width/2.0f,-height1/2.0f,0},width1*0.5f,height1,color);
    auto lineHorizontal2=ColoredGeometry::makeColoredRectangle({height1/2.0f,-height1/2.0f,0},width1*0.5f,height1,color);
    //
    const float strokeW=height1;
    auto lineVertical=ColoredGeometry::makeColoredRectangle({-strokeW/2.0f,0,0},strokeW,height,color);
    tmp.insert(tmp.end(),lineHorizontal1.begin(),lineHorizontal1.end());
    tmp.insert(tmp.end(),lineHorizontal2.begin(),lineHorizontal2.end());
    tmp.insert(tmp.end(),lineVertical.begin(),lineVertical.end());
    return ColoredMeshData(tmp,GL_TRIANGLES);
}

