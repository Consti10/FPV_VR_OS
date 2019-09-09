#ifndef VIDEORECEIVERRENDERER
#define VIDEORECEIVERRENDERER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLProgramTextureExt.h>
#include <GLProgramVC.h>
#include <android/surface_texture.h>
#include <GLProgramSpherical.h>
#include "../General/IPositionable.hpp"
#include "../General/IDrawable.hpp"
#include "../General/PositionDebug.hpp"

class VideoRenderer : public IPositionable{ //Does not inherit from IDrawable !
public:
    VideoRenderer(const GLProgramVC& glRenderGeometry,GLProgramTextureExt *glRenderTexEx,int DEV_3D_VIDEO,GLProgramSpherical *glPSpherical=nullptr);
    explicit VideoRenderer(const GLProgramVC& glRenderGeometry);
    void initUpdateTexImageJAVA(JNIEnv * env,jobject obj,jobject surfaceTexture);
    void deleteUpdateTexImageJAVA(JNIEnv* env,jobject obj); //frees the global reference so java does not complain
    void updateTexImageJAVA(JNIEnv* env);
    void punchHole(glm::mat4x4 ViewM,glm::mat4x4 ProjM);
    void punchHole2(glm::mat4x4 ViewM,glm::mat4x4 ProjM);
    void drawVideoCanvas(glm::mat4x4 ViewM, glm::mat4x4 ProjM, bool leftEye);
    void drawVideoCanvas360(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
private:
    void setupPosition() override;
    PositionDebug mPositionDebug;
    GLuint mIndexBuffer;
    GLuint mGLBuffVidLeft; //left side of the video frame (u.v coordinates)
    GLuint mGLBuffVidRight; //right side of the video frame (u.v coordinates)
    GLuint mGLBuffVid; //whole video frame (u.v coordinates)
    GLuint mGLBuffVidPunchHole;
    int nIndicesVideoCanvas;
    const GLProgramVC& mGLRenderGeometry;
    GLProgramTextureExt* mGLRenderTexEx= nullptr;
    GLProgramSpherical* mGLProgramSpherical=nullptr;

    const int TESSELATION_FACTOR=10;
    jobject localRefSurfaceTexture;
    jmethodID updateTexImageMethodId;
    jmethodID getTimestampMethodId;
    const int DEV_3D_VIDEO;
    ASurfaceTexture* mSurfaceTexture;
};

#endif
