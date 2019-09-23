//
// Created by Constantin on 12.12.2017.
//

#include <android/log.h>
#include "MatricesManager.h"
#include "MatrixHelper.h"
#include "SettingsVR.h"

constexpr auto TAG="HeadTrackerExtended";
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
constexpr auto NANO_TO_MS=1000*1000;

MatricesManager::MatricesManager(const SettingsVR& settingsVR):
        /*MODE(settingsVR.GHT_MODE),
        E_GROUND_X(settingsVR.GHT_X),
        E_GROUND_Y(settingsVR.GHT_Y),
        E_GROUND_Z(settingsVR.GHT_Z),
        IPD(settingsVR.VR_InterpupilaryDistance)*/
        settingsVR(settingsVR)
         {
   // LOGD("HT Mode %d | X %d Y %d Z %d",MODE,E_GROUND_X,E_GROUND_Y,E_GROUND_Z);
}

void MatricesManager::calculateProjectionAndDefaultView(float fov, float ratio) {
    worldMatrices.projection=glm::perspective(glm::radians(fov), ratio, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    worldMatrices.leftEyeView=glm::translate(worldMatrices.eyeView,glm::vec3(-settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    worldMatrices.rightEyeView=glm::translate(worldMatrices.eyeView,glm::vec3(settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
}

void MatricesManager::calculateProjectionAndDefaultView360(float fov360Video,float ratio) {
    worldMatrices.projection360=glm::perspective(glm::radians(fov360Video), ratio, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    worldMatrices.monoViewTracked360=glm::mat4(1.0f);
    worldMatrices.monoForward360=glm::mat4(1.0f);
    //worldMatrices.monoForward360 = glm::rotate(glm::mat4(1.0f),glm::radians(90.0f), glm::vec3(0,1,0));
}

void MatricesManager::calculateNewHeadPoseIfNeeded(gvr::GvrApi *gvr_api, const int predictMS) {
    //gvr_recti bounds=gvr_get_window_bounds(gvr_api->GetContext());
    //LOGD("bounds: %s",toString(bounds).c_str());
    if(settingsVR.GHT_MODE==MODE_NONE){
        return;
    }
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*NANO_TO_MS;
    worldMatrices.lastHeadPos=gvr_api->GetHeadSpaceFromStartSpaceRotation(target_time);
    //we only apply the neck model when we calculate the view M for 1PP
    if(settingsVR.GHT_MODE==MODE_1PP){
        gvr_api->ApplyNeckModel(worldMatrices.lastHeadPos,1);
    }
    //worldMatrices.leftEyeView=toGLM(gvr_api->GetEyeFromHeadMatrix(GVR_LEFT_EYE));
    //worldMatrices.rightEyeView=toGLM(gvr_api->GetEyeFromHeadMatrix(GVR_LEFT_EYE));

    glm::mat4x4 headView=toGLM(worldMatrices.lastHeadPos);
    headView=removeRotationAroundSpecificAxes(headView,true, true,true);

    worldMatrices.leftEyeViewTracked=glm::translate(headView,glm::vec3(-settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    worldMatrices.rightEyeViewTracked=glm::translate(headView,glm::vec3(settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    //LOGD("%s",toString(worldMatrices.leftEyeViewTracked).c_str());
}

//The 360 degree mono renderer only uses the last known head translation as 'view' matrix
void MatricesManager::calculateNewHeadPose360(gvr::GvrApi *gvr_api, const int predictMS) {
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*NANO_TO_MS;
    gvr::Mat4f tmpHeadPose = gvr_api->GetHeadSpaceFromStartSpaceRotation(target_time); //we only want rotation, screw the mirage solo
    gvr_api->ApplyNeckModel(tmpHeadPose,1);
    const glm::mat4 tmpHeadPoseGLM=toGLM(tmpHeadPose);
    worldMatrices.monoViewTracked360=tmpHeadPoseGLM*worldMatrices.monoForward360; //multiplication order is important
}

void MatricesManager::setHomeOrientation360(gvr::GvrApi *gvr_api) {
    gvr::Mat4f tmpHeadPose=gvr_api->GetHeadSpaceFromStartSpaceRotation(gvr::GvrApi::GetTimePointNow());
    gvr_api->ApplyNeckModel(tmpHeadPose,1);
    glm::mat4 headView=toGLM(tmpHeadPose);
    //headView=glm::toMat4(glm::quat_cast(headView));
    worldMatrices.monoForward360=worldMatrices.monoForward360*headView;
    //Reset tracking resets the rotation around the y axis, leaving everything else untouched
    gvr_api->RecenterTracking();
}

/*if(TEST()){
       LOGD("SUCCESS");
   }else{
       LOGD("FAIL");
   }*/