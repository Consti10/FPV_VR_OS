//
// Created by Constantin on 1/29/2019.
//

#ifndef FPV_VR_VRSETTINGS_H
#define FPV_VR_VRSETTINGS_H


#include <jni.h>
#include <array>
#include <string>
#include <vr/gvr/capi/include/gvr.h>
#include <DistortionCorrection/DistortionManager.h>

class SettingsVR {
public:
    SettingsVR(JNIEnv *env, jobject androidContext,jfloatArray undistortionData= nullptr,gvr_context* gvrContext=nullptr,bool noDistortion=false);
    SettingsVR(SettingsVR const &) = delete;
    void operator=(SettingsVR const &)= delete;
public:
    //Stereo and VR Rendering
    float VR_InterpupilaryDistance;
    float VR_SceneScale;
    int VR_DISTORTION_CORRECTION_MODE;
private:
    DistortionManager* distortionManager= nullptr;
public:
    //Might return null (if no distortion enabled)
    DistortionManager* getDistortionManager()const{
        return distortionManager;
    }
public:
    //Head tracking
    int GHT_MODE;
    bool GHT_X;
    bool GHT_Y;
    bool GHT_Z;
public:
    //std::string coeficientsToString();
};


#endif //FPV_VR_VRSETTINGS_H
