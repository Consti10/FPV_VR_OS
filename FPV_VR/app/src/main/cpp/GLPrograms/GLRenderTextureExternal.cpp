
#include "GLRenderTextureExternal.h"
#include "../Helper/GLHelper.h"

#define TAG "GLRenderTextureExternal"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define GL_TEXTURE_EXTERNAL_OES 0x00008d65

GLRenderTextureExternal::GLRenderTextureExternal(const bool distCorrection,const GLuint videoTexture) {
    distortionCorrection=distCorrection;
    if(distortionCorrection){
        mProgram = createProgram(vs_textureExt_vddc(ExampleCoeficients), fs_textureExt());
        mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
        mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    }else{
        mProgram= createProgram(vs_textureExt(), fs_textureExt());
        mMVPMatrixHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"uMVPMatrix");
    }
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTextureExt" );
    //glGenTextures(1, mTexture);
    mTexture[0]=videoTexture;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);

    checkGlError(TAG);
}

void GLRenderTextureExternal::beforeDraw(const GLuint buffer) const{
#ifdef WIREFRAME
    glLineWidth(4.0f);
#endif
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,mTexture[0]);
    glUniform1i(mSamplerHandle,1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,5*sizeof(float),(GLvoid*)(3*sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderTextureExternal::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const{
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

void GLRenderTextureExternal::afterDraw() const{
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
}
