
#ifndef COMPASLADDER
#define COMPASLADDER

#include <GLES2/gl2.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderText.h>
#include <GLRenderGeometry.h>
#include <GLRenderLine.h>

//#define DEBUG_POSITION

class CompasLadder {
private:
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    const GLRenderLine* mGLRenderLine;
    GLuint mGLLadderLinesB[1];
    GLuint mGLLadderTextB[1];
    GLuint mGLHomeArrowB[1];
    glm::mat4x4 mHeadingTranslM;
    glm::mat4x4 mHomeArrowTM;

    const int N_LINES=4*4*2; //4*4*2 Lines for the S|||| from compas
    int nChars=4*2;//8chars for N,W,S,E*2
    float degree_in_gl_translation=0;
    int charOffset=0,nCharsToDraw=0;
    int linesOffset,nLinesToDraw=0;
    struct HeadingS{
        GLuint mGLTextB[1];
        GLuint mGLOutlineLinesB[1];
        string s="init";
        int nVertices=0;
        int maxChars=4;
        float height;
    } mHeadingS;
    float mWidth,mHeight,mX,mY,mZ;
    bool eHomeArrow;
    bool invertHeading;
    float outline_quad_height;
    float outline_quad_width;
    int outline_stroke_pixels;
    int ladder_stroke_pixels;
    void updateHeadingString(float heading);
public:
    CompasLadder(const GLRenderColor* glRenderColor,const GLRenderLine* glRenderLine,const GLRenderText* glRenderText,bool eHomeArrow,bool invertHeading);
    void setWorldPosition(float x,float y,float z,float width,float height,int strokeW);
    void updateGL(float headingD,float headingHomeD);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    const float RATIO=1.0f/4.0f;
};

#endif

