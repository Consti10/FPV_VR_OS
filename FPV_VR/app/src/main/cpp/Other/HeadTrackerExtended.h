//
// Created by Constantin on 12.12.2017.
//
//This is just a wrapper around the gvrApi, since I'm using glm matrices and a custom renderer
//

#ifndef FPV_VR_HEADTRACKEREXTENDED_H
#define FPV_VR_HEADTRACKEREXTENDED_H

#include <glm/mat4x4.hpp>
#include <gvr_types.h>

/**
 * Wrapper around the gvr head tracking api
 * */

struct WorldMatrices{
    //the projection matrix. Same for left and right eye. Should be re calculated in each onSurfaceChanged via 'calculateMatrices()'
    glm::mat4x4 projection;
    //simple view matrix, without head tracking. Also recalculated in onSurfaceHanged via 'calculateMatrices()'
    //depends on the IPD selected by the user and MAX_Z_DISTANCE
    glm::mat4x4 leftEyeView;
    glm::mat4x4 rightEyeView;
    //the view matrix multiplied by the head tracking rotation matrix.
    //Recalculated with each 'updateHeadPosition()' call
    //different for 1PP and 3PP modes
    glm::mat4x4 leftEyeViewTracked; //the left eye view M with head tracking applied
    glm::mat4x4 rightEyeViewTracked; //same for left eye
};

class HeadTrackerExtended {
public:
    HeadTrackerExtended(float interpupilarryDistance); //since we use a ipd of 0 in mono mode
    static constexpr float MAX_Z_DISTANCE=14.0f;
    static constexpr int MODE_NONE=0; //No head tracking
    static constexpr int MODE_1PP=1; //first person perspective
    static constexpr int MODE_3PP_ARTIFICIALHORIZON_ONLY=2; //The head tracking on the ground only applies to the artificial horizon model.

    void calculateMatrices(float ratio);
    void calculateNewHeadPose(gvr::GvrApi* gvr_api,const int predictMS);
    WorldMatrices* getWorldMatrices();
    int mGroundTrackingMode;
    bool groundX,groundY,groundZ;
    bool airYaw,airRoll,airPitch;
    bool mEnableAirTracking;

    //void startSendingTrackingData();
    //void stopSendingTrackingData();
private:
    WorldMatrices worldMatrices;
    float IPD;
};

/*gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=8*1000*1000; //predict 8ms forward
    gvr::Mat4f tmpM;
    tmpM=gvr_api_->GetHeadSpaceFromStartSpaceRotation(target_time);
    glm::mat4x4 headView=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    headView=glm::transpose(headView);
    //
    tmpM=gvr_api_->GetEyeFromHeadMatrix(gvr::Eye::GVR_LEFT_EYE);
    glm::mat4x4 leftEyeFromHead=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    leftEyeFromHead=glm::transpose(leftEyeFromHead);
    //
    tmpM=gvr_api_->GetEyeFromHeadMatrix(gvr::Eye::GVR_RIGHT_EYE);
    glm::mat4x4 rightEyeFromHead=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    rightEyeFromHead=glm::transpose(rightEyeFromHead);

    mLeftEyeVM=headView*leftEyeFromHead;
    mRightEyeVM=headView*rightEyeFromHead;*/

/*mLeftEyeVM=glm::make_mat4(reinterpret_cast<const float*>(&head_view.m))*
        glm::make_mat4(reinterpret_cast<const float*>(&leftEyeFromHead));
mRightEyeVM=glm::make_mat4(reinterpret_cast<const float*>(&head_view.m))*
           glm::make_mat4(reinterpret_cast<const float*>(&rightEyeFromHead));
mRightEyeVM=glm::transpose(mRightEyeVM);
mLeftEyeVM=glm::transpose(mLeftEyeVM);*/
/*if(S_HeadTracking){
    HeadTrackerExtended::StereoViewM stereoViewM=mHeadTrackerExtended->getStereoViewMatrices(gvr_api_.get(),16);
    mLeftEyeVM=stereoViewM.leftEyeViewM;
    mRightEyeVM=stereoViewM.rightEyeViewM;
}*/

#endif //FPV_VR_HEADTRACKEREXTENDED_H
