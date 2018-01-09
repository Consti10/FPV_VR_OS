//
// Created by Constantin on 06.01.2018.
//

#include "GLRenderLine.h"
#include "../Helper/GLHelper.h"

GLRenderLine::GLRenderLine(const bool enableDist) {
    distortionCorrection=enableDist;
    if(distortionCorrection){
        mProgram = createProgram(vs_geometry_vddc(ExampleCoeficients), fs_geometry());
        mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
        mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    }else{
        mProgram= createProgram(vs_geometry(), fs_geometry());
        mMVPMatrixHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"uMVPMatrix");
    }
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mColorHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aColor");
    checkGlError("glGetAttribLocation OGProgramColor");
}

void GLRenderLine::beforeDraw(const GLuint buffer) const {
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, 7*4, 0);
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle, 4/*rgba*/,GL_FLOAT, GL_FALSE, 7*4,(GLvoid*)(3*4));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderLine::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset,
                        const int numberVertices,const int strokeW) const {
    if(distortionCorrection){
        glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
        glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    }else{
        glm::mat4x4 VPM= ProjM*ViewM;
        glUniformMatrix4fv(mMVPMatrixHandle, 1, GL_FALSE, glm::value_ptr(VPM));
    }
    glLineWidth(strokeW);
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
}

void GLRenderLine::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mColorHandle);
}
