//
// Created by Constantin on 8/17/2018.
//
#include "AVerticalLadder.h"
#include <Settings/SettingsOSDStyle.h>
#include <Helper/StringHelper.hpp>
#include <Helper/GLBufferHelper.hpp>
#include "Helper/GLHelper.hpp"

#define TAG "VerticalLadder"
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

AVerticalLadder::AVerticalLadder(const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms& basicGLPrograms,BatchingManager& batchingManager,const TelemetryReceiver& telemetryReceiver,
                               const bool leftHanded,const int unitsBetween,const std::wstring& unit):
        IUpdateable(TAG),IDrawable(TAG),
        settingsOSDStyle(settingsOSDStyle),
        mTelemetryReceiver(telemetryReceiver),
        LEFT_HANDED(leftHanded),
        UNITS_BETWEEN_SRINGS(unitsBetween),
        UNITS_BETWEEN_LONG_LINES(unitsBetween),
        UNIT(unit),
        mPositionDebug(basicGLPrograms.vc,0, true),
        mGLPrograms(basicGLPrograms),
        mTextObjTelemetryValue(15,false, Color::WHITE,true,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR,batchingManager),
        mBackgroundObj(batchingManager,SettingsOSDStyle::getOSDBackgroundColor(settingsOSDStyle.OSD_TRANSPARENT_BACKGROUND_STRENGTH))
{
    glGenBuffers(1,&mLadderLines.glBuffer);
    glGenBuffers(1,&mLadderStrings.glBuffer);
}


void AVerticalLadder::setupPosition() {
    mPositionDebug.setWorldPositionDebug(mX,mY,mZ,mWidth,mHeight);
    const TrueColor lineColor=settingsOSDStyle.OSD_LINE_FILL_COLOR;
    //create the background
    mBackgroundObj.setPosition(mX,mY,mZ,mWidth,mHeight);
    mBackgroundObj.recalculateData();
    //
    const float longLinesWidth=mWidth/3.0f;
    //create the ladder lines
    {
        const float distanceBetweenLines=mHeight/(4.0f*4.0f);
        const float shortLinesWidth=longLinesWidth*0.5f;
        //float tmpLinesB[7*2*N_LADDER_LINES];
        GLProgramLine::Vertex tmpLinesB[N_LADDER_LINES*6];
        for(int i=0;i<N_LADDER_LINES;i++){
            float width=shortLinesWidth;
            if(i%4==0)width=longLinesWidth;
            const float linesHeight=distanceBetweenLines*0.25f;
            if(LEFT_HANDED){
                const glm::vec3 start=glm::vec3(mX + mWidth - width,mY+ i*distanceBetweenLines,mZ);
                const glm::vec3 end=start+glm::vec3(width,0,0);
                GLProgramLine::convertLineToRenderingData(start,end,linesHeight,tmpLinesB,i*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
            }else{
                const glm::vec3 start=glm::vec3(mX,mY + i * distanceBetweenLines,mZ);
                const glm::vec3 end=start+glm::vec3(width,0,0);
                GLProgramLine::convertLineToRenderingData(start,end,linesHeight,tmpLinesB,i*6,settingsOSDStyle.OSD_LINE_FILL_COLOR,settingsOSDStyle.OSD_LINE_OUTLINE_COLOR);
            }
        }
        GLBufferHelper::allocateGLBufferStatic(mLadderLines.glBuffer,tmpLinesB,sizeof(tmpLinesB));
    }
    //place the main value element
    outlineQuadWidth=mWidth*2.0f/3.0f;
    outlineQuadHeight=outlineQuadWidth*2.0f/3.0f;
    mTextObjTelemetryValue.setTextSafe(L"INIT");
    if(LEFT_HANDED){
        mTextObjTelemetryValue.setPosition(mX,mY+mHeight/2.0f-outlineQuadHeight/2.0f,mZ,
                                           outlineQuadWidth,outlineQuadHeight);
    }else{
        mTextObjTelemetryValue.setPosition(mX+longLinesWidth,mY+mHeight/2.0f-outlineQuadHeight/2.0f,mZ,
                                           outlineQuadWidth,outlineQuadHeight);
    }
    //start with 0
    updateLadderStringsRange(0);
}

//given a middle value, load the text in the range middleValue-PRECALCULATED_RANGE_BOOTH_SIDES and middleValue+PRECALCULATED_RANGE_BOOTH_SIDES
//into OpenGL memory
//1000->0.096mByte | 10 000->0.96mByte
void AVerticalLadder::updateLadderStringsRange(int newMiddleValue) {
    GLProgramText::Character tmp[MAX_N_CHARS_PER_LADDER_STRING*N_LADDER_STRINGS];
    std::memset (&tmp, 0, sizeof(tmp));
    float ladderTextHeight=mHeight/12.0f;
    int lowestValue=newMiddleValue-PRECALCULATED_RANGE_BOOTH_SIDES;
    float distBetween=mHeight/4.0f;
    const int blub=PRECALCULATED_RANGE_BOOTH_SIDES/UNITS_BETWEEN_SRINGS;
    const auto textColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
    for(int i=0;i<N_LADDER_STRINGS;i++){
        std::wstringstream ss;
        ss<<(i*UNITS_BETWEEN_SRINGS+lowestValue);
        std::wstring s=ss.str();
        s.append(UNIT);
        if(s.length()>MAX_N_CHARS_PER_LADDER_STRING){
            s=L"E";
        }
        float x,y,z=mZ,h;
        if(LEFT_HANDED){
            float length=GLProgramText::getStringLength(s,ladderTextHeight);
            x=mX + mWidth * 2.0f / 3.0f - length;
            y=mY + mHeight / 2.0f -
              ladderTextHeight / 2.0f +
              (i - blub) * distBetween;
            h=ladderTextHeight;
        }else{
            float longLinesWidth=mWidth/3.0f;
            longLinesWidth*=0.9f; //little bit less than long lines width
            x=mX + longLinesWidth;
            y=mY + mHeight / 2.0f -
              ladderTextHeight / 2.0f +
              (i - blub) * distBetween;
            h=ladderTextHeight;
        }
        GLProgramText::convertStringToRenderingData(x,y,z,h, s,textColor, tmp,i * MAX_N_CHARS_PER_LADDER_STRING);
    }
    //LOGD("Size (in bytes) of ladder strings buffer: %d",(int)sizeof(tmp));
    GLBufferHelper::allocateGLBufferDynamic(mLadderStrings.glBuffer,tmp,sizeof(tmp));
    mLadderStrings.currentMiddleValue=newMiddleValue;
}

//update the main string that holds the accurate telemetry value
void AVerticalLadder::updateMainString(float value) {
    std::wstring beforeCome;
    std::wstring afterCome;
    std::vector<StylizedString> ss;
    const auto textColor=settingsOSDStyle.OSD_TEXT_FILL_COLOR2;
    if(abs(value)<10){
        StringHelper::doubleToString(beforeCome,afterCome,value,6,2);
        ss={{beforeCome,1.0f,textColor},{afterCome,0.5f,textColor}};

    }else if(abs(value)<100){
        StringHelper::doubleToString(beforeCome,afterCome,value,5,1);
        ss={{beforeCome,0.8f,textColor},{afterCome,0.5f,textColor}};
    }else{
        auto s=StringHelper::intToString((int)value,8);
        float scale=OSDTextObj::calculateBiggestFittingScale(s,outlineQuadHeight,outlineQuadWidth);
        scale*=0.95f; //because the outline needs some pixels too
        ss={{s,scale,textColor}};
    }
    mTextObjTelemetryValue.setTextSafe(ss);
    mTextObjTelemetryValue.recalculateDataIfNeeded();
}

//calculate the offset & translation matrix for the ladder lines
void AVerticalLadder::calcLadderLinesRenderData(const float value) {
    float glTranslationPerUnit=mHeight/80;
    float valueModPos=fmod(value,(float)UNITS_BETWEEN_LONG_LINES);
    if(valueModPos<0)valueModPos+=UNITS_BETWEEN_LONG_LINES;
    mLadderLines.currentDrawOffset=((int)valueModPos/5+1);
    mLadderLines.currentDrawNumber=(N_LADDER_LINES-4);
    mLadderLinesTM = glm::translate(glm::mat4(1.0f),glm::vec3(0,-glTranslationPerUnit*valueModPos,0));
}

//calculate the offsets & translation matrix for the ladder strings
void AVerticalLadder::calcLadderStringsRenderData(const float value) {
    float glTranslationPerUnit=mHeight/(UNITS_BETWEEN_SRINGS*4);
    int currentDrawOffset=N_LADDER_STRINGS/2-1;
    currentDrawOffset+=(int)(value-mLadderStrings.currentMiddleValue)/UNITS_BETWEEN_SRINGS;
    if(value<0){
        currentDrawOffset-=1;
    }
    //now we have to find the string that is occluded by the telemetry value element
    float valueModPos=fmod(value,(float)UNITS_BETWEEN_SRINGS);
    if(valueModPos<0)valueModPos+=UNITS_BETWEEN_SRINGS;
    if(valueModPos>UNITS_BETWEEN_SRINGS/2.0f){
        mLadderStrings.currentDrawOffset1=currentDrawOffset;
        mLadderStrings.currentDrawNumber1=2;
        mLadderStrings.currentDrawOffset2=currentDrawOffset+3;
        mLadderStrings.currentDrawNumber2=1;
    }else{
        mLadderStrings.currentDrawOffset1=currentDrawOffset;
        mLadderStrings.currentDrawNumber1=1;
        mLadderStrings.currentDrawOffset2=currentDrawOffset+2;
        mLadderStrings.currentDrawNumber2=2;
    }
    //also the translation m
    mLadderStrings.currTranslationM = glm::translate(glm::mat4(1.0f),
                                                     glm::vec3(0,-glTranslationPerUnit*(value-mLadderStrings.currentMiddleValue),0));
}

void AVerticalLadder::updateGL() {
    //see if we have to recalculate the ladder strings
    int currentMinimum=mLadderStrings.currentMiddleValue-PRECALCULATED_RANGE_BOOTH_SIDES;
    int currentMaximum=mLadderStrings.currentMiddleValue+PRECALCULATED_RANGE_BOOTH_SIDES;
    int newMiddleValue;
    if(verticalLadderValue-40<=currentMinimum || verticalLadderValue+40>=currentMaximum){
        newMiddleValue=roundToMultiple((int)verticalLadderValue,PRECALCULATED_RANGE_BOOTH_SIDES);
        LOGD("Recalculation needed. height m: %d | new middle value: %d",(int)verticalLadderValue,newMiddleValue);
        updateLadderStringsRange(newMiddleValue);
    }
    updateMainString(verticalLadderValue);
    calcLadderLinesRenderData(verticalLadderValue);
    calcLadderStringsRenderData(verticalLadderValue);
}

void AVerticalLadder::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    mPositionDebug.drawGLDebug(ViewM,ProjM);

    //Background is drawn by BatchingManager
    //Main telemetry element is drawn by batchingManager

    //draw the ladder lines
    mGLPrograms.line.beforeDraw(mLadderLines.glBuffer);
    mGLPrograms.line.draw(mLadderLinesTM*ViewM, ProjM, mLadderLines.currentDrawOffset*6,mLadderLines.currentDrawNumber*6);
    mGLPrograms.line.afterDraw();

    //draw the ladder strings
    mGLPrograms.text.beforeDraw(mLadderStrings.glBuffer);
    mGLPrograms.text.draw(mLadderStrings.currTranslationM*ViewM,ProjM,
                           6*MAX_N_CHARS_PER_LADDER_STRING*mLadderStrings.currentDrawOffset1,
                           6*MAX_N_CHARS_PER_LADDER_STRING*mLadderStrings.currentDrawNumber1);
    mGLPrograms.text.draw(mLadderStrings.currTranslationM*ViewM,ProjM,
                           6*MAX_N_CHARS_PER_LADDER_STRING*mLadderStrings.currentDrawOffset2,
                           6*MAX_N_CHARS_PER_LADDER_STRING*mLadderStrings.currentDrawNumber2);
    mGLPrograms.text.afterDraw();
}

