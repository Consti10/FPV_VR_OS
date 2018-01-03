
#include "GLRenderColor.h"
#include "../Helper/GLHelper.h"

#define TAG "GLRenderColor"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

GLRenderColor::GLRenderColor(bool enableDist) {
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

void GLRenderColor::beforeDraw(const GLuint buffer) const {
#ifdef WIREFRAME
    glLineWidth(4.0f);
#endif
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, 7*4, 0);
    glEnableVertexAttribArray((GLuint)mColorHandle);
    glVertexAttribPointer((GLuint)mColorHandle, 4/*rgba*/,GL_FLOAT, GL_FALSE, 7*4,(GLvoid*)(3*4));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderColor::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const{
    if(distortionCorrection){
        glUniformMatrix4fv(mMVMatrixHandle, 1, GL_FALSE, glm::value_ptr(ViewM));
        glUniformMatrix4fv(mPMatrixHandle, 1, GL_FALSE, glm::value_ptr(ProjM));
    }else{
        glm::mat4x4 VPM= ProjM*ViewM;
        glUniformMatrix4fv(mMVPMatrixHandle, 1, GL_FALSE, glm::value_ptr(VPM));
    }
#ifdef WIREFRAME
    glDrawArrays(GL_LINES, verticesOffset, numberVertices);
    glDrawArrays(GL_POINTS, verticesOffset, numberVertices);
#endif
#ifndef WIREFRAME
    glDrawArrays(GL_TRIANGLES, verticesOffset, numberVertices);
#endif
}

void GLRenderColor::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mColorHandle);
}
