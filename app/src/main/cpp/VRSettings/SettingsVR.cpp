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

    VR_InterpupilaryDistance=settingsN.getFloat(IDVR::VR_InterpupilaryDistance,0.2f);
    VR_SceneScale=settingsN.getFloat(IDVR::VR_SceneScale,50.0f);
    VR_DISTORTION_CORRECTION_MODE=settingsN.getInt(IDVR::VR_DISTORTION_CORRECTION_MODE);

    GHT_MODE=settingsN.getInt(IDVR::GroundHeadTrackingMode);
    GHT_X=settingsN.getBoolean(IDVR::GroundHeadTrackingX);
    GHT_Y=settingsN.getBoolean(IDVR::GroundHeadTrackingY);
    GHT_Z=settingsN.getBoolean(IDVR::GroundHeadTrackingZ);

    //VR_DISTORTION_CORRECTION_MODE=2;
    distortionManager=nullptr;

    //LOGD("Coeficients %s",coeficientsToString().c_str());
    //LOGD("HT Mode %d | X %d Y %d Z %d",GHT_MODE,GHT_X,GHT_Y,GHT_Z);
    LOGD("%f %f %d",VR_InterpupilaryDistance,VR_SceneScale,VR_DISTORTION_CORRECTION_MODE);
}


/*std::string SettingsVR::coeficientsToString() {
    std::stringstream ss;
    ss<<VR_DC_UndistortionData[0]<<", "<<VR_DC_UndistortionData[1]<<", "<<VR_DC_UndistortionData[2]<<", "<<VR_DC_UndistortionData[3]<<", "<<VR_DC_UndistortionData[4]<<", ";
    ss<<VR_DC_UndistortionData[5]<<", "<<VR_DC_UndistortionData[6]<<", "<<VR_DC_UndistortionData[7];
    return ss.str();
}
*/



/*void VRSettingsHelper::refresh(JNIEnv *env, jobject androidContext) {
    VRSettingsHelper& instance=VRSettingsHelper::getInstance();
    SettingsN settingsN(env,androidContext,"pref_vr_rendering");

    instance.VR_InterpupilaryDistance=settingsN.getFloat(IDVR::VR_InterpupilaryDistance);
    instance.VR_SceneScale=settingsN.getFloat(IDVR::VR_SceneScale);
    instance.VR_DistortionCorrection=settingsN.getBoolean(IDVR::VR_DistortionCorrection);

    instance.GHT_MODE=settingsN.getInt(IDVR::GroundHeadTrackingMode);
    instance.GHT_X=settingsN.getBoolean(IDVR::GroundHeadTrackingX);
    instance.GHT_Y=settingsN.getBoolean(IDVR::GroundHeadTrackingY);
    instance.GHT_Z=settingsN.getBoolean(IDVR::GroundHeadTrackingZ);

    LOGD("%f %f %d", instance.VR_InterpupilaryDistance,instance.VR_SceneScale,(int)instance.VR_DistortionCorrection);

}


#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_Settings_SettingsVR_##method_name

extern "C" {

JNI_METHOD(void, refresh)
(JNIEnv *env, jclass unused, jobject androidContext) {
    VRSettingsHelper::getInstance().refresh(env, androidContext);
}

JNI_METHOD(void, setUndistortionData)
(JNIEnv *env, jclass unused, jfloat maxRSq, jfloat k1, jfloat k2, jfloat k3, jfloat k4, jfloat k5,
 jfloat k6//,jfloat k7,jfloat k8) {
    VRSettingsHelper &instance = VRSettingsHelper::getInstance();
    instance.VR_DC_UndistortionData[0] = maxRSq;
    instance.VR_DC_UndistortionData[1] = k1;
    instance.VR_DC_UndistortionData[2] = k2;
    instance.VR_DC_UndistortionData[3] = k3;
    instance.VR_DC_UndistortionData[4] = k4;
    instance.VR_DC_UndistortionData[5] = k5;
    instance.VR_DC_UndistortionData[6] = k6;
}

}*/
