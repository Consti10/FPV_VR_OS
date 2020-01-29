//
// Created by Constantin on 8/20/2018.
//

#include "VLSpeed.h"
#include <SettingsOSDStyle.h>


VLSpeed::VLSpeed(const VLSpeed::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms &basicGLPrograms, BatchingManager &batchingManager,
                               const TelemetryReceiver &telemetryReceiver) :
        mOptions(options),
        AVerticalLadder(settingsOSDStyle,basicGLPrograms, batchingManager, telemetryReceiver,true,10,
                       options.useKMHinsteadOfMS ? L"km/h" : L"m/s"){}


void VLSpeed::updateGL() {
    float speedKMH;
    const UAVTelemetryData& uav_td= mTelemetryReceiver.getUAVTelemetryData();
    speedKMH=uav_td.SpeedGround_KPH;
    if(mOptions.useKMHinsteadOfMS){
        verticalLadderValue=speedKMH;
    }else{
        verticalLadderValue=speedKMH*1000.0f/60.0f/60.0f;
    }
    AVerticalLadder::updateGL();
}

IPositionable::Rect2D VLSpeed::calculatePosition(const IPositionable::Rect2D &osdOverlay,const bool stereo) {
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
    float x=osdOverlay.mX+osdOverlay.mWidth*OFFSET_VIDEO_X;
    float y=osdOverlay.mY+(osdOverlay.mHeight-height)/2.0f;
    float z=osdOverlay.mZ;
    return {x,y,z,width,height};
}
