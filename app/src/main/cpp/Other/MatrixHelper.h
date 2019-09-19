//
// Created by Consti10 on 18/09/2019.
//

#ifndef FPV_VR_2018_MATRIXHELPER_H
#define FPV_VR_2018_MATRIXHELPER_H

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <sstream>


//1)gvr matrices are stored in row major order
//See gvr_types.h:
/// A floating point 4x4 matrix stored in row-major form. It needs to be
/// transposed before being used with OpenGL.
//2)glm matrices are stored in column major order (glm manual,section 4.11), as well as OpenGL matrices

static glm::mat4 toGLM(const gvr::Mat4f &gvrMatrix){
    glm::mat4x4 ret=glm::make_mat4(reinterpret_cast<const float*>(&gvrMatrix.m));
    ret=glm::transpose(ret);
    return ret;
}

static gvr::Mat4f toGVR(const glm::mat4 &glmMatrix){
    glm::mat4 glmMatrixTransposed=glm::transpose(glmMatrix);
    gvr::Mat4f ret=gvr::Mat4f();
    memcpy(ret.m,glm::value_ptr(glmMatrixTransposed),4*4*sizeof(float));
    return ret;
}

static gvr::Mat4f MatrixMul(const glm::mat4x4 &m1, const gvr::Mat4f &m2){
    glm::mat4 m2AsGLM=toGLM(m2);
    return toGVR(m1*m2AsGLM);
}

//remove rotation around specific axes, but leaves
//all other axes & translations intact
//We can lock head tracking on specific axises. Therefore, we first calculate the inverse quaternion from the headView (rotation) matrix.
//when we multiply the inverse and the headView at the end they cancel each other out.
//Therefore, for each axis we want to use we set the rotation on the inverse to zero.
static glm::mat4 removeRotationAroundSpecificAxes(const glm::mat4 mat,const bool xRotEnable,const bool yRotEnable,const bool zRotEnable){
    if(xRotEnable && yRotEnable && zRotEnable)
        return mat;
    glm::quat quatRotInv = glm::inverse(glm::quat_cast(mat));
    glm::vec3 euler=glm::eulerAngles(quatRotInv);
    if(xRotEnable){
        euler.x=0;
    }
    if(yRotEnable){
        euler.y=0;
    }
    if(zRotEnable){
        euler.z=0;
    }
    glm::quat quat=glm::quat(euler);
    glm::mat4x4 RotationMatrix = glm::toMat4(quat);
    return mat*RotationMatrix;
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

static float toRadians(double degree){
    return (float)(degree * 3.141592653589793F / 180.0f);
}


/*static glm::mat4 toGLM(const gvr::Mat4f &matrix) {
    glm::mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result[i][j] = matrix.m[i][j];
        }
    }
    result=glm::transpose(result);
    return result;
}*/

/*gvr::Mat4f MatrixMul(const glm::mat4x4 &m1, const gvr::Mat4f &m2) {
    glm::mat4x4 tm2=glm::make_mat4(reinterpret_cast<const float*>(&m2.m));
    tm2 = m1 * tm2;
    gvr::Mat4f ret;
    memcpy(reinterpret_cast<float*>(&ret.m), reinterpret_cast<float*>(&tm2[0]), 16 * sizeof(float));
    return ret;
}*/

//float aaa[16]={1,-1.94429e-07,5.55111e-17,0,
//               1.94429e-07,1,1.66533e-16,0,
//               -5.55112e-17,-1.66533e-16,1,0,
//               -0.1,1.94429e-08,-5.55111e-18,1,};
//worldMatrices.monoViewTracked360=glm::make_mat4x4(aaa);
/*float rot=0;
void MatricesManager::calculateNewHeadPose360_fixRot(gvr::GvrApi *gvr_api, const int predictMS) {
    gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
    target_time.monotonic_system_time_nanos+=predictMS*NANO_TO_MS;
    gvr::Mat4f tmpM = gvr_api->GetHeadSpaceFromStartSpaceTransform(target_time);
    tmpM = MatrixMul(worldMatrices.monoForward360, tmpM);
    gvr_api->ApplyNeckModel(tmpM,1);
    glm::mat4x4 headView=glm::make_mat4(reinterpret_cast<const float*>(&tmpM.m));
    headView=glm::transpose(headView);
    worldMatrices.monoViewTracked360=glm::translate(headView,glm::vec3(0,0,0.0));
    //worldMatrices.monoViewTracked360=glm::mat4();
    //worldMatrices.monoViewTracked360=glm::rotate(worldMatrices.monoViewTracked360,rot,glm::vec3(0,0,1));
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


#endif //FPV_VR_2018_MATRIXHELPER_H
