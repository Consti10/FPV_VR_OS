/*****************************************************
 * Abstraction for drawing geometry in form of x,y,z, u,v (texture coordinates) vertices
 * A Instance of this class also refers to a specific TextureUnit (e.g. Texture_0,Texture_1) once bound
 * a Image can be loaded into this texture via loadTextureImage()
 * The application only has to create and fill a VBO with matching data.
 * Setting up the rendering variables and rendering is done via beforeDraw(),draw() and afterDraw().
 ****************************************************/
#ifndef GLRENDERTEXTURE
#define GLRENDERTEXTURE

#include <GLES2/gl2.h>
#include <glm/mat4x4.hpp>
#include <jni.h>
#include "android/log.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLRenderTexture {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mMVPMatrixHandle,mSamplerHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint mTexture[1];
public:
    bool distortionCorrection;
    GLRenderTexture(const bool distCorrection);
    void loadTextureImage(JNIEnv *env, jobject obj);
    void beforeDraw(const GLuint buffer) const;
    void draw(const glm::mat4x4 ViewM, const glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices) const;
    void afterDraw() const;
};

#endif
