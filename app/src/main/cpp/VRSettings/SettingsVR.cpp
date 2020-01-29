//
// Created by Constantin on 1/29/2019.
//

#include <android/log.h>
#include <array>
#include <sstream>
#include <Helper/SharedPreferences.hpp>
#include <Helper/MDebug.hpp>
#include "SettingsVR.h"
#include "IDVR.hpp"

SettingsVR::SettingsVR(JNIEnv *env, jobject androidContext) {
    SharedPreferences settingsN(env,androidContext,"pref_vr_rendering");

    VR_SCENE_SCALE_PERCENTAGE=settingsN.getFloat(IDVR::VR_SCENE_SCALE_PERCENTAGE,100.0F);
    //Default on (0==0ff, 1==On)
    VR_DISTORTION_CORRECTION_MODE=settingsN.getInt(IDVR::VR_DISTORTION_CORRECTION_MODE,1);

    GHT_MODE=settingsN.getInt(IDVR::GroundHeadTrackingMode);
    bool headTracking=GHT_MODE!=0;
    GHT_X=settingsN.getBoolean(IDVR::GroundHeadTrackingX) && headTracking;
    GHT_Y=settingsN.getBoolean(IDVR::GroundHeadTrackingY) && headTracking;
    GHT_Z=settingsN.getBoolean(IDVR::GroundHeadTrackingZ) && headTracking;

    LOGD("%f %d",VR_SCENE_SCALE_PERCENTAGE,VR_DISTORTION_CORRECTION_MODE);
}


