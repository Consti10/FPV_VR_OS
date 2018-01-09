/***********************************************************
 * Abstraction for drawing Text.
 * The Application has to
 * 1)load the Texture font atlas into texture memory via "loadTextureImage"
 * 2) create and fill a GLBuffer with vertex/uv/color data. This can be done using convertStringToVECs_UVs
 * 3) draw the Text by calling beforeDraw(),draw() and afterDraw()
 * an exampe for calling first a red, then a green string;
 *  a)beforeDraw(mGLBuffer);
 *  b)draw(...,r=1.0,g=0.0,b=0.0)
 *  b)draw(...,r=0.0,g=1.0,b=0.0)
 *  c)afterDraw();
 *
 *  Note: When drawing dynamically changing text you should consider multithreading and asynchronous uploading of vecs_uvs data, like
 *  I did in OSD/TextElements.cpp
 * */

#ifndef GLRENDERTEXT
#define GLRENDERTEXT

#include <GLES2/gl2.h>
#include "android/log.h"
#include "GLTextObj.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <jni.h>
#include <string>

using namespace std;

class GLRenderText {
private:
    GLuint mProgram;
    GLint mPositionHandle,mTextureHandle,mMVPMatrixHandle,mSamplerHandle,mColorHandle;
    GLuint mMVMatrixHandle,mPMatrixHandle;
    GLuint mTexture[1];
    bool distortionCorrection;
public:
    GLRenderText(const bool distCorrection);
    void loadTextureImage(JNIEnv *env, jobject obj,jobject assetManagerJAVA);
    static int convertStringToVECs_UVs(const string text, const float X, const float Y, const float Z,const float charHeight,float array[],const int arrayOffset);
    static void convertStringToVECs_UVs(GLTextObj* stringObject);
    static float getStringLength(string s,float scale);
    void beforeDraw(const GLuint buffer) const;
    void draw(const glm::mat4x4 ViewM, const  glm::mat4x4 ProjM, const int verticesOffset, const int numberVertices,const float r,const float g,const float b) const;
    void afterDraw() const;
};

#endif
