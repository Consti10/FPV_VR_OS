/*******************************************************************
 * Abstraction for drawing an android surface as an OpenGL Texture
 *******************************************************************/
#ifndef GLRENDERTEXTUREEXTERNAL
#define GLRENDERTEXTUREEXTERNAL

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLRenderTextureExternal {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mMVPMatrixHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint mTexture[1];
public:
    bool distortionCorrection;
    GLRenderTextureExternal(const bool distCorrection,const GLuint videoTexture);
    void beforeDraw(const GLuint buffer) const;
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const;
    void afterDraw() const;
};

#endif
