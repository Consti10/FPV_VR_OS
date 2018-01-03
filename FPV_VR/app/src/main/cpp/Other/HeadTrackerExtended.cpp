//
// Created by Constantin on 12.12.2017.
//

#include "HeadTrackerExtended.h"
#include "gvr.h"
#include "gvr_types.h"
#include "../SettingsN.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.inl>
#include <android/log.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>

#define TAG "HeadTrackerExtended"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

HeadTrackerExtended::HeadTrackerExtended(float interpupilarryDistance) {
    IPD=interpupilarryDistance;
    this->mGroundTrackingMode=S_GHT_MODE;
    this->mEnableAirTracking=S_AHT_MODE==0;
    groundX= S_GHT_X;
    groundY=S_GHT_Y;
    groundZ= S_GHT_Z;

    airYaw=S_AHT_YAW;
    airRoll=S_AHT_ROLL;
    airPitch=S_AHT_PITCH;
}

void HeadTrackerExtended::calculateMatrices(float ratio) {
    worldMatrices.projection=glm::perspective(glm::radians(45.0f), ratio, 0.1f, MAX_Z_DISTANCE+5.0f);
    worldMatrices.leftEyeView=glm::lookAt(
            glm::vec3(0,0,0),
            glm::vec3(0,0,-MAX_Z_DISTANCE),
            glm::vec3(0,1,0)
    );
    worldMatrices.rightEyeView=glm::lookAt(
            glm::vec3(0,0,0),
            glm::vec3(0,0,-MAX_Z_DISTANCE),
            glm::vec3(0,1,0)
    );
    glm::translate(worldMatrices.leftEyeView,glm::vec3(-IPD/2.0f,0,0));
    glm::translate(worldMatrices.rightEyeView,glm::vec3(IPD/2.0f,0,0));
    /*worldMatrices.leftEyeView=glm::lookAt(
            glm::vec3(0,0,0),
            glm::vec3(-S_InterpupilaryDistance/2.0f,0,-MAX_Z_DISTANCE),
            glm::vec3(0,1,0)
    );
    worldMatrices.rightEyeView=glm::lookAt(
            glm::vec3(0,0,0),
            glm::vec3(S_InterpupilaryDistance/2.0f,0,-MAX_Z_DISTANCE),
            glm::vec3(0,1,0)
    );*/
}

void HeadTrackerExtended::calculateNewHeadPose(gvr::GvrApi *gvr_api, const int predictMS) {
    if(mGroundTrackingMode==MODE_NONE){
        LOGV("cannot calculate new head pose. False tracking mode.");
        return;
    }
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*1000*1000;
    gvr::Mat4f tmpM;
    tmpM=gvr_api->GetHeadSpaceFromStartSpaceRotation(target_time);
    //we only apply the neck model when we calculate the view M for 1PP
    if(mGroundTrackingMode==MODE_1PP){
        gvr_api->ApplyNeckModel(tmpM,1);
    }
    glm::mat4x4 headView=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    headView=glm::transpose(headView);
    //We can lock head tracking on specific axises. Therefore, we first calculate the inverse quaternion from the headView (rotation) matrix.
    //when we multiply the inverse and the headView at the end they cancel each other out.
    //Therefore, for each axis we want to use we set the rotation on the inverse to zero.
    if(!groundX || !groundY || !groundZ){
        glm::quat quatRotInv = glm::inverse(glm::quat_cast(headView));
        glm::vec3 euler=glm::eulerAngles(quatRotInv);
        if(groundX){
            euler.x=0;
        }
        if(groundY){
            euler.y=0;
        }
        if(groundZ){
            euler.z=0;
        }
        glm::quat quat=glm::quat(euler);
        glm::mat4x4 RotationMatrix = glm::toMat4(quat);
        headView=headView*RotationMatrix;
    }
    worldMatrices.leftEyeViewTracked=glm::translate(headView,glm::vec3(-IPD/2.0f,0,0));
    worldMatrices.rightEyeViewTracked=glm::translate(headView,glm::vec3(IPD/2.0f,0,0));
}

WorldMatrices* HeadTrackerExtended::getWorldMatrices() {
    return &worldMatrices;
}

/*glm::mat4x4 HeadTrackerExtended::getHeadSpaceFromStartSpaceRotation(gvr::GvrApi* gvr_api,int predictMS) {
    return glm::mat4x4();
}

HeadTrackerExtended::StereoViewM HeadTrackerExtended::getStereoViewMatrices(gvr::GvrApi* gvr_api,const int predictMS,const bool applyNeckModel) {
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*1000*1000;
    gvr::Mat4f tmpM;
    StereoViewM stereoViewM;

    tmpM=gvr_api->GetHeadSpaceFromStartSpaceRotation(target_time);
    if(applyNeckModel){
        gvr_api->ApplyNeckModel(tmpM,1);
    }

    glm::mat4x4 headView=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    headView=glm::transpose(headView);
    //glm::quat mQuat=glm::toQuat(headView);
    //LOGV("");
    //
    tmpM=gvr_api->GetEyeFromHeadMatrix(gvr::Eye::GVR_LEFT_EYE);
    glm::mat4x4 leftEyeFromHead=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    leftEyeFromHead=glm::transpose(leftEyeFromHead);
    //
    tmpM=gvr_api->GetEyeFromHeadMatrix(gvr::Eye::GVR_RIGHT_EYE);
    glm::mat4x4 rightEyeFromHead=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    rightEyeFromHead=glm::transpose(rightEyeFromHead);

    //stereoViewM.leftEyeViewM=headView*leftEyeFromHead;
    //stereoViewM.rightEyeViewM=headView*rightEyeFromHead;
    stereoViewM.leftEyeViewM=headView;
    stereoViewM.rightEyeViewM=headView*rightEyeFromHead;
    /*glm::mat4x4 mLeftEyeVM=glm::lookAt(
            glm::vec3(-0.2f/2.0f,0,0),
            glm::vec3(0,0,-14.0f),
            glm::vec3(0,1,0)
    );
    stereoViewM.leftEyeViewM=headView*mLeftEyeVM;
    stereoViewM.leftEyeViewM=headView;*/
    /*return stereoViewM;
}*/
/*glm::quat quatRotation = glm::quat_cast(headView);
    quatRotation=glm::inverse(quatRotation);
    glm::mat4x4 RotationMatrix = glm::toMat4(quatRotation);
    headView=headView*RotationMatrix;*/

