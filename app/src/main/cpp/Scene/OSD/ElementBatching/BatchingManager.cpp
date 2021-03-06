//
// Created by Constantin on 8/23/2018.
//

#include "BatchingManager.h"

BatchingManager::BatchingManager(const BasicGLPrograms &basicGLPrograms):
        mBasicGLPrograms(basicGLPrograms),
        mBufferVCTriangles("Background"),
        mBufferVCLines("Outline"),
        mBufferText("Text")//,mBufferLines("Lines")
        {
}

std::shared_ptr<ModifiableArray<ColoredVertex>> BatchingManager::allocateVCTriangles(const int nVertices) {
    return mBufferVCTriangles.allocate(nVertices);
}

std::shared_ptr<ModifiableArray<ColoredVertex>> BatchingManager::allocateVCLines(const int nVertices) {
    return mBufferVCLines.allocate(nVertices);
}

std::shared_ptr<ModifiableArray<GLProgramText::Character>> BatchingManager::allocateText(const int nRectangles) {
    return mBufferText.allocate(nRectangles);
}

//std::shared_ptr<ModifiableArray<GLProgramLine::Vertex>> BatchingManager::allocateLines(int nLines) {
//    return mBufferLines.allocate(nLines*6);
//}


void BatchingManager::setTextColor(const TrueColor textOutlineColor,const float textOutlineStrength) {
    mTextOutlineColor=textOutlineColor;
    mTextOutlineStrength=textOutlineStrength;
}

void BatchingManager::initGL() {
    mBufferVCTriangles.setupGPUBuffer();
    mBufferText.setupGPUBuffer();
    mBufferVCLines.setupGPUBuffer();
    //mBufferLines.setupGPUBuffer();
}

void BatchingManager::updateGL() {
    mBufferVCTriangles.uploadToGpuIfModified();
    mBufferText.uploadToGpuIfModified();
    mBufferVCLines.uploadToGpuIfModified();
    //mBufferLines.uploadToGpuIfModified();
}

void BatchingManager::drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM) {
    //draw the background before the text
    const int nTriangleVertices=(int)mBufferVCTriangles.nElements;
    mBasicGLPrograms.vc.beforeDraw(mBufferVCTriangles.gpuBuffer);
    mBasicGLPrograms.vc.draw(ViewM,ProjM,0,nTriangleVertices,GL_TRIANGLES);
    mBasicGLPrograms.vc.afterDraw();

    const int nTextVertices= (int)mBufferText.nElements * 6;
    mBasicGLPrograms.text.beforeDraw(mBufferText.gpuBuffer);
    mBasicGLPrograms.text.updateOutline(mTextOutlineColor, mTextOutlineStrength);
    mBasicGLPrograms.text.draw(ProjM,0,nTextVertices);
    mBasicGLPrograms.text.afterDraw();

    const int nOutlineVertices=(int)mBufferVCLines.nElements;
    mBasicGLPrograms.vc.beforeDraw(mBufferVCLines.gpuBuffer);
    glLineWidth(1);
    mBasicGLPrograms.vc.draw(ViewM,ProjM,0,nOutlineVertices,GL_LINES);
    mBasicGLPrograms.vc.afterDraw();

    /*mBasicGLPrograms.line.beforeDraw(mBufferLines.gpuBuffer);
    mBasicGLPrograms.line.draw(ViewM,ProjM,0,(int)mBufferLines.nElements);
    mBasicGLPrograms.line.afterDraw();*/

    //LOGD("N vertices: background: %d | text %d | outline %d",nTriangleVertices,nTextVertices,nOutlineVertices);

    /*GLfloat lineWidthRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_LINE_WIDTH, lineWidthRange);
    LOGD("non aliased: %f %f",lineWidthRange[0],lineWidthRange[1]);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    LOGD("Aliased: %f %f",lineWidthRange[0],lineWidthRange[1]);*/
}


