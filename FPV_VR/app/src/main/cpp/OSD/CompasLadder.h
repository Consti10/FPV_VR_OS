
#ifndef COMPASLADDER
#define COMPASLADDER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderText.h>
#include <GLRenderColor.h>

//#define DEBUG_POSITION

class CompasLadder {
public:
    CompasLadder(const GLRenderColor* glRenderColor,const GLRenderText* glRenderText,bool eHomeArrow,bool invertHeading);
    void setWorldPosition(float x,float y,float z,float width,float height,float strokeW);
    void updateGL(float headingD,float headingHomeD);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    const float RATIO=1.0f/4.0f;
private:
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    GLuint mGLLines_HomeArrowB[1];
    GLuint mGLTextB1[1];
    glm::mat4x4 mCompasTM;
    glm::mat4x4 mHomeArrowTM;

    int nLines=4*4*2; //4*4*2 Lines for the S|||| from compas
    int nChars=4*2;//8chars for N,W,S,E*2
    float degree_in_gl_translation=0;
    int charOffset=0,nCharsToDraw=0;
    int linesOffset,nLinesToDraw=0;
    struct HeadingS{
        GLuint mGLBuffer[1];
        string s="init";
        int nVertices=0;
        int maxChars=4;
        float height;
    } mHeadingS;
    float mWidth,mHeight,mX,mY,mZ;
    bool eHomeArrow;
    bool invertHeading;
    float artMovC=0;
    bool artMovReverse= false;
};

#endif

