//
// Created by Constantin on 8/23/2018.
//

#ifndef FPV_VR_DYNAMICBACKGROUNDMANAGER_H
#define FPV_VR_DYNAMICBACKGROUNDMANAGER_H


#include <General/IUpdateable.hpp>
#include <General/IDrawable.hpp>
#include <BasicGLPrograms.hpp>
#include <TrueColor.hpp>
#include "GLHelper.hpp"
#include "CpuGpuBuff.h"

#include <GLES2/gl2.h>
#include <array>
#include <memory>
/**
 * Batching many elements of the same kind into one draw call gives a big performance improvement
 * To use the batching manager, use "allocateXXX" to get a buffer and use modify() to write into this buffer
 * Each time per frame, the batching manager checks if these buffer(s) have been modified and uploads the new content to the GPU if needed
 * I then draws all the vertices in one single draw call
 */
class BatchingManager : public IUpdateable, public IDrawable {
public:
    explicit BatchingManager(const BasicGLPrograms& basicGLPrograms);
    std::shared_ptr<ModifiableArray<ColoredVertex>> allocateVCTriangles(int nVertices);
    std::shared_ptr<ModifiableArray<ColoredVertex>> allocateVCLines(int nVertices);
    std::shared_ptr<ModifiableArray<GLProgramText::Character>> allocateText(int nRectangles);
    //
    std::shared_ptr<ModifiableArray<GLProgramLine::Vertex>> allocateLines(int nLines);
    //
    void initGL();
    void updateGL()override;
    void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM)override;
    void setTextColor(TrueColor textOutlineColor,float textOutlineStrength);
private:
    const BasicGLPrograms& mBasicGLPrograms;
    TrueColor mTextOutlineColor;
    float mTextOutlineStrength;
    CpuGpuBuff<ColoredVertex> mBufferVCTriangles;
    CpuGpuBuff<ColoredVertex> mBufferVCLines;
    CpuGpuBuff<GLProgramText::Character> mBufferText;
    //
    CpuGpuBuff<GLProgramLine::Vertex> mBufferLines;
};


#endif //FPV_VR_DYNAMICBACKGROUNDMANAGER_H
