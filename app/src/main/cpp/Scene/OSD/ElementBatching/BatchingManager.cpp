//
// Created by Constantin on 8/23/2018.
//

#include "BatchingManager.h"
#include <GLHelper.hpp>

#include <GLES3/gl3.h>
#include <TrueColor.hpp>
#include <BasicGLPrograms.hpp>


#define TAG "BatchingManager"
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

BatchingManager::BatchingManager(const BasicGLPrograms &basicGLPrograms):
        mBasicGLPrograms(basicGLPrograms),
        mTriangleBuffer("Background"),
        mOutlineB("Outline"),
        mTextB("Text"){
}

ModifiableArray<GLProgramVC::Vertex>* BatchingManager::allocateVCTriangles(const int nVertices) {
    return mTriangleBuffer.allocate(nVertices);
}

ModifiableArray<GLProgramVC::Vertex> *BatchingManager::allocateVCLines(const int nVertices) {
    return mOutlineB.allocate(nVertices);
}

ModifiableArray<GLProgramText::Character> *BatchingManager::allocateText(const int nRectangles) {
    return mTextB.allocate(nRectangles);
}

void BatchingManager::setTextColor(const TrueColor textOutlineColor,const float textOutlineStrength) {
    mTextOutlineColor=textOutlineColor;
    mTextOutlineStrength=textOutlineStrength;
}

void BatchingManager::initGL() {
    mTriangleBuffer.setupGPUBuffer();
    mTextB.setupGPUBuffer();
    mOutlineB.setupGPUBuffer();
}

void BatchingManager::updateGL() {
    mTriangleBuffer.uploadToGpuIfModified();
    mTextB.uploadToGpuIfModified();
    mOutlineB.uploadToGpuIfModified();
}

void BatchingManager::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    //draw the background before the text
    const int nTriangleVertices=mTriangleBuffer.size;
    mBasicGLPrograms.vc.beforeDraw(mTriangleBuffer.gpuBuffer);
    mBasicGLPrograms.vc.draw(glm::value_ptr(ViewM),glm::value_ptr(ProjM),0,nTriangleVertices,GL_TRIANGLES);
    mBasicGLPrograms.vc.afterDraw();

    const int nTextVertices=mTextB.size*6;
    mBasicGLPrograms.text.beforeDraw(mTextB.gpuBuffer);
    mBasicGLPrograms.text.updateOutline(mTextOutlineColor, mTextOutlineStrength);
    mBasicGLPrograms.text.draw(ViewM,ProjM,0,nTextVertices);
    mBasicGLPrograms.text.afterDraw();

    const int nOutlineVertices=mOutlineB.size;
    mBasicGLPrograms.vc.beforeDraw(mOutlineB.gpuBuffer);
    glLineWidth(1);
    mBasicGLPrograms.vc.draw(glm::value_ptr(ViewM),glm::value_ptr(ProjM),0,nOutlineVertices,GL_LINES);
    mBasicGLPrograms.vc.afterDraw();

    //LOGD("N vertices: background: %d | text %d | outline %d",nTriangleVertices,nTextVertices,nOutlineVertices);

    /*GLfloat lineWidthRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_LINE_WIDTH, lineWidthRange);
    LOGD("non aliased: %f %f",lineWidthRange[0],lineWidthRange[1]);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    LOGD("Aliased: %f %f",lineWidthRange[0],lineWidthRange[1]);*/
}

