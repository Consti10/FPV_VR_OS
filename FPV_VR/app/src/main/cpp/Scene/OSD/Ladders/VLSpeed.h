//
// Created by Constantin on 8/20/2018.
//

#ifndef FPV_VR_SPEEDLADDER_H
#define FPV_VR_SPEEDLADDER_H

#include "AVerticalLadder.h"

class VLSpeed : public AVerticalLadder{
public:
    struct Options {
        bool enable = true;
        int scale=100;
        bool useKMHinsteadOfMS=false;
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
