//
// Created by Constantin on 12.12.2017.
//
//This is just a wrapper around the gvrApi, since I'm using glm matrices and a custom renderer
//

#ifndef FPV_VR_HEADTRACKEREXTENDED_H
#define FPV_VR_HEADTRACKEREXTENDED_H

#include <glm/mat4x4.hpp>
#include "vr/gvr/capi/include/gvr_types.h"
#include "SettingsVR.h"

/**
 * Wrapper around the gvr head tracking api
 * that holds most commonly used matrices /
 * matrices that are needed for OpenGL rendering
 * */


struct Matrices{
    //the projection matrix. Same for left and right eye. Should be re calculated in each onSurfaceChanged via 'calculateProjectionAndDefaultView()'
    glm::mat4x4 projection;
    //the view matrix, without any IPD
    glm::mat4x4 eyeView;
    //simple view matrices, without head tracking, but IPD. Also recalculated in onSurfaceHanged via 'calculateProjectionAndDefaultView()'
    //depends on the IPD set on initialisation
    glm::mat4x4 leftEyeView;
    glm::mat4x4 rightEyeView;
    //the view matrix multiplied by the head tracking rotation matrix.
    //Recalculated with each 'updateHeadPosition()' call
    //different for 1PP and 3PP modes
    glm::mat4x4 leftEyeViewTracked; //the left eye view M with head tracking applied
    glm::mat4x4 rightEyeViewTracked; //same for left eye

    gvr::Mat4f lastHeadPos;
    //360
    glm::mat4x4 monoViewTracked360;
    // The view matrix the defines the forward orientation
    // This should be added into the rotation matrix
    glm::mat4x4 monoForward360;
    glm::mat4x4 projection360;
};

class MatricesManager {
public:
    explicit MatricesManager(const SettingsVR& settingsVR);
    static constexpr float MAX_Z_DISTANCE=20.0f;
    static constexpr float MIN_Z_DISTANCE=1.0f;
    static constexpr int MODE_NONE=0; //No head tracking
    static constexpr int MODE_1PP=1; //first person perspective
    void calculateProjectionAndDefaultView(float fov, float ratio);
    void calculateProjectionAndDefaultView360(float fov360Video,float ratio);
    void calculateNewHeadPoseIfNeeded(gvr::GvrApi *gvr_api, int predictMS);
    void calculateNewHeadPose360(gvr::GvrApi* gvr_api,const int predictMS);
    Matrices& getWorldMatrices(){ return worldMatrices;};
    const SettingsVR& settingsVR;
    void setHomeOrientation360(gvr::GvrApi *gvr_api);
private:
    Matrices worldMatrices;
};


#endif //FPV_VR_HEADTRACKEREXTENDED_H
