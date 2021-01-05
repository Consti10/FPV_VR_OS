//
// Created by Constantin on 8/20/2018.
//

#ifndef FPV_VR_XXXLADDER_H
#define FPV_VR_XXXLADDER_H


#include "AVerticalLadder.h"


class VLAltitude: public AVerticalLadder {
public:
    enum SOURCE_VAL{BARO,GPS};
    struct Options {
        bool enable = true;
        int scale=100;
        SOURCE_VAL sourceVal=BARO;
    };
public:
    VLAltitude(const VLAltitude::Options& options,const SettingsOSDStyle& settingsOSDStyle,const BasicGLPrograms &basicGLPrograms, BatchingManager &batchingManager,
              const TelemetryReceiver &telemetryReceiver);
    Rect2D calculatePosition(const Rect2D &osdOverlay, bool stereo);
    //static constexpr float OFFSET_VIDEO_X=17.0f/20.0f;
private:
    void updateGL() override;
    const VLAltitude::Options& mOptions;
};


#endif //FPV_VR_XXXLADDER_H
