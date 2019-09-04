//
// Created by Constantin on 8/10/2018.
//
#include "TEWarning.h"
#include <Settings/SettingsOSDStyle.h>
#define TAG "TEWarning"

TEWarning::TEWarning(const TEWarning::Options& options,const BasicGLPrograms &basicGLPrograms,BatchingManager& batchingManager,TelemetryReceiver &telemetryReceiver):
        IUpdateable(TAG), IDrawable(TAG),
        mTelemetryReceiver(telemetryReceiver),
        mPositionDebug(basicGLPrograms.vc,2, true),
        mOptions(options),
        mGLTextObjIndices(OSDTextObj::createAll(N_CHARS_PER_TEXT_OBJ,MAX_N_TEXT_OBJ,true,
                                                Color::fromRGBA(0, 0, 0, 0.3f),false,
                                                Color::WHITE,batchingManager)) {
}

void TEWarning::setupPosition() {
    mPositionDebug.setWorldPositionDebug(mX,mY,mZ,mWidth,mHeight);
    float width=mWidth;
    float height=mHeight/3.0f;
    for(int i=0;i<MAX_N_TEXT_OBJ;i++){
        auto& obj=mGLTextObjIndices.at((unsigned long)i);
        obj->setPosition(mX,mY+mHeight-(i+1)*height,mZ,width,height);
        obj->setBounds(OSDTextObj::MIDDLE);
        obj->recalculateDataIfNeeded();
    }
}

void TEWarning::updateGL() {
    //every x ms, disable all elements
    int64_t timeMS=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    bool blinkCounter=std::sin(timeMS/100)>0.5f;
    if(blinkCounter){
        for(auto& i:mGLTextObjIndices){
            i->setTextSafe(L"");
            i->recalculateDataIfNeeded();
        }
        return;
    }
    //Batt %
    auto obj=mGLTextObjIndices.at(0).get();
    if(mOptions.batteryPercentage &&
       mTelemetryReceiver.getTelemetryValue(TelemetryReceiver::BATT_PERCENTAGE).warning>0){
        obj->setTextSafe(L"BATT %", Color::RED);
    }else{
        obj->setTextSafe(L"");
    }
    obj->recalculateDataIfNeeded();
    //Batt V
    obj=mGLTextObjIndices.at(1).get();
    if(mOptions.batteryVoltage &&
       mTelemetryReceiver.getTelemetryValue(TelemetryReceiver::BATT_VOLTAGE).warning>0){
        obj->setTextSafe(L"BATT V", Color::RED);
    }else{
        obj->setTextSafe(L"");
    }
    obj->recalculateDataIfNeeded();
    //Batt mAh used
    obj=mGLTextObjIndices.at(2).get();
    if(mOptions.batteryMAHUsed &&
       mTelemetryReceiver.getTelemetryValue(TelemetryReceiver::BATT_USED_CAPACITY).warning>0){
        obj->setTextSafe(L"BATT mAh", Color::RED);
    }else{
        obj->setTextSafe(L"");
    }
    obj->recalculateDataIfNeeded();
}

void TEWarning::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    mPositionDebug.drawGLDebug(ViewM,ProjM);
}

IPositionable::Rect2D TEWarning::calculatePosition(const IPositionable::Rect2D &osdOverlay) {
    float width=GLProgramText::getStringLength(MAX_TEXT_LENGTH_REFERENCE,mTextHeight);
    float height=mTextHeight*MAX_N_TEXT_OBJ;
    float clHeight=mTextHeight*3;
    float x=osdOverlay.mX+osdOverlay.mWidth/2.0f-width*0.5f;
    float y=osdOverlay.mY+osdOverlay.mHeight-height-clHeight;
    float z=osdOverlay.mZ;
    return {x,y,z,width,height};
}
