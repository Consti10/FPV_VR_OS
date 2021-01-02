
#include "CompassLadder.h"
#include <SettingsOSDStyle.h>
#include <GLHelper.hpp>

#define TAG "CompassLadder"

CompassLadder::CompassLadder(CompassLadder::Options options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryReceiver(telemetryReceiver),
        mGLPrograms(basicGLPrograms),
        mOptions(options),
        mTextObjTelemetryValue(N_CHARS_PER_TEXT_OBJ,false,TrueColor2::WHITE,true,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR,batchingManager),
        mBackgroundObj(batchingManager,SettingsOSDStyle::getOSDBackgroundColor(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH)),
        mMiddleArrow(batchingManager.allocateVCTriangles(3)){
}


void CompassLadder::setupPosition() {
    mCalcTextHeight=mHeight/3.0f;
    const auto lineColor=settingsOSDStyle.OSD_LINE_FILL_COLOR;
    const auto textColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
    degreeInGLTranslation=mWidth/360.0f;
    float d_between_lines=mWidth/16.0f;
//Place the current heading element
    {
        float currentHeadingElementHeight=mCalcTextHeight*TEXT_UPSCALE;
        float currentHeadingElementWidth=currentHeadingElementHeight*2.5f;
        //OSDTextObj15* obj=mTextManager.getOSDTextObj15(mGLTextObjIndices.at(0));
        mTextObjTelemetryValue.setTextSafe(L"INIT");
        mTextObjTelemetryValue.setBounds(OSDTextObj::MIDDLE);
        mTextObjTelemetryValue.setPosition(mX+mWidth/2.0f-currentHeadingElementWidth/2.0f,mY+mHeight-currentHeadingElementHeight,
                         currentHeadingElementWidth,currentHeadingElementHeight);
        mTextObjTelemetryValue.recalculateDataIfNeeded();
    }
//make the ladder lines
    float longLinesHeight=mCalcTextHeight*0.8f;
    {
        const float ladderLinesHeight=mCalcTextHeight*0.2f*3.5f;
        const float ladderLinesWidth=ladderLinesHeight*0.3f;
        float ladderLinesStartY=mY+mHeight-mCalcTextHeight*TEXT_UPSCALE-ladderLinesHeight;//in relation to the top !
        std::vector<GLProgramLine::Vertex> tmp(N_EXISTING_LADDER_LINES*6);
        glm::vec3 color=glm::vec3(1,1,1);
        int offset=0;
        for(int i=0;i<N_EXISTING_LADDER_LINES;i+=4){
            glm::vec3 start;
            start=glm::vec3(mX + (0 + i) * d_between_lines,ladderLinesStartY,0);
            GLProgramLine::convertLineToRenderingData(start,start+glm::vec3(0,ladderLinesHeight,0),ladderLinesWidth,tmp.data(),offset++*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
            start=glm::vec3(mX + (1 + i) * d_between_lines,ladderLinesStartY, 0);
            GLProgramLine::convertLineToRenderingData(start,start+glm::vec3(0,ladderLinesHeight,0),ladderLinesWidth,tmp.data(),offset++*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
            start=glm::vec3(mX + (2 + i) * d_between_lines,ladderLinesStartY, 0);
            GLProgramLine::convertLineToRenderingData(start,start+glm::vec3(0,ladderLinesHeight,0),ladderLinesWidth,tmp.data(),offset++*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
            start=glm::vec3(mX + (3 + i) * d_between_lines,ladderLinesStartY, 0);
            GLProgramLine::convertLineToRenderingData(start,start+glm::vec3(0,ladderLinesHeight,0),ladderLinesWidth,tmp.data(),offset++*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
        }
        mGLLadderLinesB.uploadGL(tmp);
    }
//create the S W N E chars
    {
        float nwseCharsHeight=mCalcTextHeight*TEXT_UPSCALE;
        float nwseCharsStartY=mY+mHeight-mCalcTextHeight*TEXT_UPSCALE-nwseCharsHeight*1.1f;
        std::vector<GLProgramText::Character> tmp(N_EXISTING_CHARS);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"S",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 0,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"S",
                                                    textColor,
                                                    tmp.data(), 0);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"W",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 1,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"W",
                                                    textColor,
                                                    tmp.data(), 1);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"N",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 2,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"N",
                                                    textColor,
                                                    tmp.data(), 2);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"E",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 3,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"E",
                                                    textColor,
                                                    tmp.data(), 3);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"S",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 4,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"S",
                                                    textColor,
                                                    tmp.data(), 4);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"W",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 5,
                                                    nwseCharsStartY,0, nwseCharsHeight, L"W",
                                                    textColor,
                                                    tmp.data(), 5);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"N",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 6,
                                                    nwseCharsStartY,0, nwseCharsHeight, L"N",
                                                    textColor,
                                                    tmp.data(), 6);
        GLProgramText::convertStringToRenderingData(mX - GLProgramText::getStringLength(L"E",
                                                                                        nwseCharsHeight) /
                                                         2.0f + d_between_lines * 4 * 7,
                                                    nwseCharsStartY, 0, nwseCharsHeight, L"E",
                                                    textColor,
                                                    tmp.data(), 7);
        mGLLadderTextB.uploadGL(tmp);
    }
//create the home symbol icon
    {
        std::vector<GLProgramText::Character> tmp(1);
        float home_arrow_width_height=mHeight/3.0f;
        float haStartY=mY+mHeight-mCalcTextHeight*TEXT_UPSCALE-longLinesHeight-home_arrow_width_height;
        std::wstring wstring1;
        wstring1+=GLProgramText::ICON_HOME;
        GLProgramText::convertStringToRenderingData(mX - home_arrow_width_height / 2.0f,
                                                    haStartY, 0,
                                                    home_arrow_width_height, wstring1,
                                                    TrueColor2::GREEN, tmp.data(), 0);
        mGLHomeIconB.uploadGL(tmp);
    }
    //create the middle arrow
    {
        float middle_arrow_width_height=mHeight/8.0f;
        const glm::vec3 p1(mX+mWidth/2.0f,mY+mHeight-mCalcTextHeight*TEXT_UPSCALE-middle_arrow_width_height/2.0f,0);
        const glm::vec3 p2(p1.x+middle_arrow_width_height,p1.y+middle_arrow_width_height,p1.z);
        const glm::vec3 p3(p1.x-middle_arrow_width_height,p1.y+middle_arrow_width_height,p1.z);
        ColoredGeometry::makeColoredTriangle(mMiddleArrow->modify(),p1,p2,p3,lineColor,lineColor,lineColor);
    }
//create the background rectangle
    mBackgroundObj.setPosition(mX,mY,mWidth,mHeight);
    mBackgroundObj.recalculateData();
}


void CompassLadder::updateGL() {
    float heading_Deg;;
    float headingHome_Deg;
    if(mOptions.COGoverMag){
        heading_Deg=mTelemetryReceiver.getCourseOG_Deg();
    }else{
        heading_Deg=mTelemetryReceiver.getHeading_Deg();
    }
    if(mOptions.invHeading){
        heading_Deg*=-1;
    }
    headingHome_Deg=mTelemetryReceiver.getHeadingHome_Deg();
    //now validate all values btw. bring them into the right range
    heading_Deg=std::fmod(heading_Deg,360.0f);
    headingHome_Deg=std::fmod(headingHome_Deg,360.0f);
    if(heading_Deg<0){
        heading_Deg+=360;
    }
    if(headingHome_Deg<0){
        headingHome_Deg+=360;
    }
    {
        const auto mColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
        std::wstringstream ss;
        ss<<(int)round(heading_Deg);
        mTextObjTelemetryValue.setTextSafe(ss.str() + L"°", mColor);
        mTextObjTelemetryValue.recalculateDataIfNeeded();
    }
    linesOffset=0;
    float tmpHeading=heading_Deg;
    while(tmpHeading>0){
        tmpHeading-=22.5f;
        linesOffset+=1;
    }
    nLinesToDraw=16;
    charOffset=0;
    tmpHeading=heading_Deg;
    while(tmpHeading>0){
        tmpHeading-=90;
        charOffset+=1;
    }
    nCharsToDraw=4;
    mHeadingTranslM = glm::translate(glm::mat4(1.0f),glm::vec3(-degreeInGLTranslation*heading_Deg,0,0));
    if(mOptions.homeArrow){
        float homeTransl=(headingHome_Deg+180)-heading_Deg;
        homeTransl=std::fmod(homeTransl,360.0f);
        if(homeTransl<0){
            homeTransl+=360;
        }
        mHomeArrowTM = glm::translate(glm::mat4(1.0f), glm::vec3(degreeInGLTranslation*homeTransl,0,0));
    }
}

void CompassLadder::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {

    //Background is drawn by BatchingManager
    //Main telemetry element is drawn by batchingManager
    //Middle arrow is drawn by BatchingManager

    //Render all the ladder lines
    mGLPrograms.line.beforeDraw(mGLLadderLinesB.getGLBufferId());
    mGLPrograms.line.draw(ViewM*mHeadingTranslM,ProjM,6*linesOffset,6*nLinesToDraw);
    mGLPrograms.line.afterDraw();

    //Render the N W S E chars
    mGLPrograms.text.beforeDraw(mGLLadderTextB.getGLBufferId());
    mGLPrograms.text.draw(ProjM*mHeadingTranslM,6*charOffset,6*nCharsToDraw);
    mGLPrograms.text.afterDraw();

    //Render the home icon
    mGLPrograms.text.beforeDraw(mGLHomeIconB.getGLBufferId());
    if(mOptions.homeArrow){
        mGLPrograms.text.draw(ProjM*mHomeArrowTM,0,6);
    }
    mGLPrograms.text.afterDraw();
}

IPositionable::Rect2D CompassLadder::calculatePosition(const IPositionable::Rect2D &osdOverlay,const bool stereo) {
    float percentageVideoY;
    float GLOBAL_SCALE;
    if(stereo){
        percentageVideoY=PERCENTAGE_VIDEO_Y_STEREO;
        GLOBAL_SCALE=settingsOSDStyle.OSD_STEREO_GLOBAL_SCALE;
    }else{
        percentageVideoY=PERCENTAGE_VIDEO_Y_MONO;
        GLOBAL_SCALE=settingsOSDStyle.OSD_MONO_GLOBAL_SCALE;
    }
    float height=osdOverlay.mHeight*percentageVideoY*(mOptions.scale/100.0f)*(GLOBAL_SCALE*0.01f);
    float width=height*RATIO;
    float x=osdOverlay.mX+(osdOverlay.mWidth-width)*0.5f;
    float y=osdOverlay.mY+osdOverlay.mHeight-height;
    return {x,y,width,height};
}
