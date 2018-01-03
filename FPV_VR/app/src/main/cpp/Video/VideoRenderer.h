
#ifndef VIDEORECEIVERRENDERER
#define VIDEORECEIVERRENDERER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderTextureExternal.h>
#include <GLRenderColor.h>

//#define DEBUG_POSITION


class VideoRenderer {
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
public:
    VideoRenderer(GLRenderColor *glRenderColor,GLRenderTextureExternal *glRenderTexEx,bool eTesselation);
    void setWorldPosition(float videoX,float videoY,float videoZ,float videoW,float videoH);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    void initUpdateTexImageJAVA(JNIEnv * env,jobject obj,jobject surfaceTexture);
    void deleteUpdateTexImageJAVA(JNIEnv* env,jobject obj); //frees the global reference so java does not complain
    void updateTexImageJAVA(JNIEnv* env);
    //void drawStereoGL(glm::mat4x4 leftVM,glm::mat4x4 rightVM,glm::mat4x4 ProjM,int vpW,int vpH);
private:
    GLuint mGLBuffer[1];
    GLRenderColor* mGLRenderColor;
    GLRenderTextureExternal* mGLRenderTexEx;
    bool enableTesselation;
    int nVertices;
    int TesselationFactor=20;

    jobject localRefSurfaceTexture;
    jmethodID updateTexImageMethodId;
    jmethodID getTimestampMethodId;
};

#endif
