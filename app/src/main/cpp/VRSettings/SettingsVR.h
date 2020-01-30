//
// Created by Constantin on 1/29/2019.
//

#ifndef FPV_VR_VRSETTINGS_H
#define FPV_VR_VRSETTINGS_H


#include <jni.h>
#include <array>
#include <string>
#include <vr/gvr/capi/include/gvr.h>

class SettingsVR {
public:
    SettingsVR(JNIEnv *env, jobject androidContext);
    SettingsVR(SettingsVR const &) = delete;
    void operator=(SettingsVR const &)= delete;
public:
    //Stereo and VR Rendering
    int VR_DISTORTION_CORRECTION_MODE;
    float VR_SCENE_SCALE_PERCENTAGE;
public:
    //Head tracking
    int GHT_MODE;
    bool GHT_X;
    bool GHT_Y;
    bool GHT_Z;
    //
    bool GHT_OSD_FIXED_TO_HEAD;
public:
    static constexpr const float DEFAULT_FOV_FILLED_BY_SCENE=60;
};


#endif //FPV_VR_VRSETTINGS_H
