//
// Created by Constantin on 8/20/2018.
//

#ifndef FPV_VR_SPEEDLADDER_H
#define FPV_VR_SPEEDLADDER_H

#include "AVerticalLadder.h"

// Shows the horizontal speed of the aircraft if enabled
// (NOT VS, but HS==Horizontal Speed). Only the orientation of the ladder is vertical
class VLSpeed : public AVerticalLadder{
public:
    struct Options {
        bool enable = true;
        int scale=100;
    };
public:
    VLSpeed(const VLSpeed::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms &basicGLPrograms, BatchingManager &batchingManager,
                   const TelemetryReceiver &telemetryReceiver);
    Rect2D calculatePosition(const Rect2D &osdOverlay,bool stereo);
    static constexpr float OFFSET_VIDEO_X=3.0f/20.0f;
private:
    void updateGL() override;
    const VLSpeed::Options& mOptions;
};


#endif //FPV_VR_SPEEDLADDER_H
