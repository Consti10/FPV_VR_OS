//
// Created by Constantin on 1/29/2019.
//

#ifndef FPV_VR_VRSETTINGS_H
#define FPV_VR_VRSETTINGS_H


#include <jni.h>
#include <array>
#include <string>

class SettingsVR {
public:
    SettingsVR(JNIEnv *env, jobject androidContext,jfloatArray undistortionData= nullptr);
    SettingsVR(SettingsVR const &) = delete;
    void operator=(SettingsVR const &)= delete;
public:
    //Stereo and VR Rendering
    float VR_InterpupilaryDistance;
    float VR_SceneScale;
    bool VR_DistortionCorrection;
    std::array<float,7> VR_DC_UndistortionData;
public:
    //Head tracking
    int GHT_MODE;
    bool GHT_X;
    bool GHT_Y;
    bool GHT_Z;
public:
    int DEV_3D_VIDEO=0;
public:
    std::string coeficientsToString();
};


#endif //FPV_VR_VRSETTINGS_H
