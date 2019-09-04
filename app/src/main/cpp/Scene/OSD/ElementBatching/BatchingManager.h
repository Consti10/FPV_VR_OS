//
// Created by Constantin on 8/23/2018.
//

#ifndef FPV_VR_DYNAMICBACKGROUNDMANAGER_H
#define FPV_VR_DYNAMICBACKGROUNDMANAGER_H


#include <General/IUpdateable.hpp>
#include <General/IDrawable.hpp>
#include <GLES2/gl2.h>
#include <array>
#include <Helper/BasicGLPrograms.hpp>
#include "Helper/GLHelper.hpp"

#include "CpuGpuBuff.h"


class BatchingManager : public IUpdateable, public IDrawable {
public:
    explicit BatchingManager(const BasicGLPrograms& basicGLPrograms);
    ModifiableArray<GLProgramVC::Vertex>* allocateVCTriangles(int nVertices);
    ModifiableArray<GLProgramVC::Vertex>* allocateVCLines(int nVertices);
    ModifiableArray<GLProgramText::Character>* allocateText(int nRectangles);
    void initGL();
    void updateGL()override;
    void drawGL(const glm::mat4x4& ViewM,const glm::mat4x4& ProjM)override;
    void setTextColor(TrueColor textOutlineColor,float textOutlineStrength);
private:
    const BasicGLPrograms& mBasicGLPrograms;
    TrueColor mTextOutlineColor;
    float mTextOutlineStrength;
    CpuGpuBuff<GLProgramVC::Vertex> mTriangleBuffer;
    CpuGpuBuff<GLProgramVC::Vertex> mOutlineB;
    CpuGpuBuff<GLProgramText::Character> mTextB;
};


#endif //FPV_VR_DYNAMICBACKGROUNDMANAGER_H
