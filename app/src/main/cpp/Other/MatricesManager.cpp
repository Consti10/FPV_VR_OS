//
// Created by Constantin on 12.12.2017.
//

#include "MatricesManager.h"
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "SettingsVR.h"
#include "Helper/GLHelper.hpp"

//#define TAG "HeadTrackerExtended"
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
constexpr auto NANO_TO_MS=1000*1000;

static glm::mat4 toGLM(const gvr::Mat4f &matrix) {
    glm::mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = matrix.m[i][j];
        }
    }
    result=glm::transpose(result);
    return result;
}
gvr::Mat4f MatrixMul(const glm::mat4x4 &m1, const gvr::Mat4f &m2) {
    glm::mat4x4 tm2=glm::make_mat4(reinterpret_cast<const float*>(&m2.m));
    tm2 = m1 * tm2;
    gvr::Mat4f ret;
    memcpy(reinterpret_cast<float*>(&ret.m), reinterpret_cast<float*>(&tm2[0]), 16 * sizeof(float));
    return ret;
}

static const std::string toString(const glm::mat4x4 matrix){
    std::stringstream ss;
    ss<<"\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            ss<<matrix[i][j]<<",";
        }
        ss<<"\n";
    }
    return ss.str();
}


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

static float toRadians(double degree){
    return (float)(degree * 3.141592653589793F / 180.0f);
}

void MatricesManager::calculateMatrices(float fov,float ratio) {
    worldMatrices.projection=glm::perspective(glm::radians(fov), ratio, MIN_Z_DISTANCE, MAX_Z_DISTANCE);
    worldMatrices.leftEyeView=glm::translate(worldMatrices.eyeView,glm::vec3(-settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    worldMatrices.rightEyeView=glm::translate(worldMatrices.eyeView,glm::vec3(settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    //360
    const float S_FieldOfView=40;
    worldMatrices.projection360=glm::perspective(glm::radians(S_FieldOfView), ratio, 0.1f, MAX_Z_DISTANCE+5);
    worldMatrices.monoViewTracked=glm::mat4();
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
    //We can lock head tracking on specific axises. Therefore, we first calculate the inverse quaternion from the headView (rotation) matrix.
    //when we multiply the inverse and the headView at the end they cancel each other out.
    //Therefore, for each axis we want to use we set the rotation on the inverse to zero.
    /*{
        glm::quat quatRotInv = glm::inverse(glm::quat_cast(headView));
        glm::vec3 euler=glm::eulerAngles(quatRotInv);
        if(!E_GROUND_X){
            euler.x=0;
        }
        if(!groundY){
            euler.y=0;
        }
        if(!E_GROUND_Z){
            euler.z=0;
        }
        glm::quat quat=glm::quat(euler);
        glm::mat4x4 RotationMatrix = glm::toMat4(quat);
        headView=headView*RotationMatrix;
    }*/
    worldMatrices.leftEyeViewTracked=glm::translate(headView,glm::vec3(-settingsVR.VR_InterpupilaryDistance/2.0f,0,0));
    worldMatrices.rightEyeViewTracked=glm::translate(headView,glm::vec3(settingsVR.VR_InterpupilaryDistance/2.0f,0,0));

    //LOGD("%s",toString(worldMatrices.leftEyeViewTracked).c_str());
}

//The 360 degree mono renderer only uses the last known head translation as 'view' matrix
void MatricesManager::calculateNewHeadPose360(gvr::GvrApi *gvr_api, const int predictMS) {
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*NANO_TO_MS;
    const gvr::Mat4f tmpHeadPose = gvr_api->GetHeadSpaceFromStartSpaceTransform(target_time);
    gvr_api->ApplyNeckModel(tmpHeadPose,1);
    worldMatrices.monoViewTracked=toGLM(tmpHeadPose);
}


//float aaa[16]={1,-1.94429e-07,5.55111e-17,0,
//               1.94429e-07,1,1.66533e-16,0,
//               -5.55112e-17,-1.66533e-16,1,0,
//               -0.1,1.94429e-08,-5.55111e-18,1,};
//worldMatrices.monoViewTracked=glm::make_mat4x4(aaa);
/*float rot=0;
void MatricesManager::calculateNewHeadPose360_fixRot(gvr::GvrApi *gvr_api, const int predictMS) {
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*NANO_TO_MS;
    gvr::Mat4f tmpM = gvr_api->GetHeadSpaceFromStartSpaceTransform(target_time);
    tmpM = MatrixMul(worldMatrices.monoForward360, tmpM);
    gvr_api->ApplyNeckModel(tmpM,1);
    glm::mat4x4 headView=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    headView=glm::transpose(headView);
    worldMatrices.monoViewTracked=glm::translate(headView,glm::vec3(0,0,0.0));
    //worldMatrices.monoViewTracked=glm::mat4();
    //worldMatrices.monoViewTracked=glm::rotate(worldMatrices.monoViewTracked,rot,glm::vec3(0,0,1));
    //rot+=0.02f;

}*/

/*fov=100;
    float left=fov/2.0f;
    float right=fov/2.0f;
    float top=fov/2.0f;
    float bottom=fov/2.0f;
    float near=MIN_Z_DISTANCE;
    float far=MAX_Z_DISTANCE;
    float l = (float)(-std::tan(toRadians((double)left))) * near;
    float r = (float)std::tan(toRadians((double)right)) * near;
    float b = (float)(-std::tan(toRadians((double)bottom))) * near;
    float t = (float)std::tan(toRadians((double)top)) * near;
    worldMatrices.projection=glm::frustum(l, r, b, t, near, far);*/