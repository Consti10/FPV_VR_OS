
#ifndef MODELROLLPITCH
#define MODELROLLPITCH

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderGeometry.h>

//#define DEBUG_POSITION

class ModelRollPitch {
private:
    float mX,mY,mZ;
    float mModelWidth;
    const GLRenderColor* mGLRenderColor;
    GLuint GLBuffer[1];
    glm::mat4x4 mModelM;
    bool roll,pitch,invRoll,invPitch;
    float artRoll=0,artPitch=0;
    bool artMovReverse=false;
    bool artRollReverse=false,artPitchReverse=false;
public:
    ModelRollPitch(const GLRenderColor *glRenderGeometry,bool roll,bool pitch,bool invRoll,bool invPitch);
    void setWorldPosition(float x, float y, float z,float modelWidth);
    void update(float rollDegree, float pitchDegree);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM,bool create3PP);
};

#endif