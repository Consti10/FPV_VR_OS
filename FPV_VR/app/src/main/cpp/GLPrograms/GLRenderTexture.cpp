
#include "GLRenderTexture.h"
#include "../Helper/GLHelper.h"

#define TAG "GLRenderTexture"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


GLRenderTexture::GLRenderTexture(const bool distCorrection) {
    distortionCorrection=distCorrection;
    if(distortionCorrection){
        mProgram = createProgram(vs_texture_vddc(ExampleCoeficients), fs_texture());
        mMVMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uMVMatrix");
        mPMatrixHandle=(GLuint)glGetUniformLocation(mProgram,"uPMatrix");
    }else{
        mProgram= createProgram(vs_texture(), fs_texture());
        mMVPMatrixHandle=(GLuint)glGetUniformLocation((GLuint)mProgram,"uMVPMatrix");
    }
    mPositionHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aPosition");
    mTextureHandle = (GLuint)glGetAttribLocation((GLuint)mProgram, "aTexCoord");
    mSamplerHandle = glGetUniformLocation (mProgram, "sTexture" );
    glGenTextures(1, mTexture);
    checkGlError("Error GLRenderTexture");
}

void GLRenderTexture::beforeDraw(const GLuint buffer) const{
#ifdef WIREFRAME
    glLineWidth(4.0f);
#endif
    glUseProgram((GLuint)mProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,mTexture[0]);
    glUniform1i(mSamplerHandle,1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mPositionHandle);
    glVertexAttribPointer((GLuint)mPositionHandle, 3/*3vertices*/, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray((GLuint)mTextureHandle);
    glVertexAttribPointer((GLuint)mTextureHandle, 2/*uv*/,GL_FLOAT, GL_FALSE,5*sizeof(float),(GLvoid*)(3*sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderTexture::draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const {
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

void GLRenderTexture::afterDraw() const {
    glDisableVertexAttribArray((GLuint)mPositionHandle);
    glDisableVertexAttribArray((GLuint)mTextureHandle);
    glBindTexture(GL_TEXTURE_2D,0);
}

void GLRenderTexture::loadTextureImage(JNIEnv *env, jobject obj) {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,mTexture[0]);
    jclass jclass1 = env->FindClass("constantin/osdtester/GLRenderer14_Stereo");
    if(jclass1== 0){
        LOGV("Zero");
    }
    jmethodID loadImageIntoTexture=env->GetStaticMethodID(jclass1, "loadImageIntoTexture", "(Ljava/lang/String;)V");
    jstring jstr = env->NewStringUTF( "drawable/texture");
    env->CallStaticVoidMethod(jclass1, loadImageIntoTexture,jstr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("loadTexture");
}
