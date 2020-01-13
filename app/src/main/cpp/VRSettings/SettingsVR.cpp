//
// Created by Constantin on 1/29/2019.
//

#include <android/log.h>
#include <array>
#include <sstream>
#include <Helper/SharedPreferences.hpp>
#include "SettingsVR.h"
#include "IDVR.hpp"

SettingsVR::SettingsVR(JNIEnv *env, jobject androidContext,jfloatArray radialUndistData,gvr_context* gvrContext,bool noDistortion) {
    SharedPreferences settingsN(env,androidContext,"pref_vr_rendering");

    VR_SCENE_SCALE_PERCENTAGE=settingsN.getFloat(IDVR::VR_SCENE_SCALE_PERCENTAGE,100.0F);
    VR_DISTORTION_CORRECTION_MODE=settingsN.getInt(IDVR::VR_DISTORTION_CORRECTION_MODE);

    GHT_MODE=settingsN.getInt(IDVR::GroundHeadTrackingMode);
    bool headTracking=GHT_MODE!=0;
    GHT_X=settingsN.getBoolean(IDVR::GroundHeadTrackingX) && headTracking;
    GHT_Y=settingsN.getBoolean(IDVR::GroundHeadTrackingY) && headTracking;
    GHT_Z=settingsN.getBoolean(IDVR::GroundHeadTrackingZ) && headTracking;

    //LOGD("Coeficients %s",coeficientsToString().c_str());
    //LOGD("HT Mode %d | X %d Y %d Z %d",GHT_MODE,GHT_X,GHT_Y,GHT_Z);
    LOGD("%f %d",VR_SCENE_SCALE_PERCENTAGE,VR_DISTORTION_CORRECTION_MODE);
}


