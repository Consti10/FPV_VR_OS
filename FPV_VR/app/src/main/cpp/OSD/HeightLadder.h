
#ifndef HEIGHTLADDER
#define HEIGHTLADDER

#include <glm/mat4x4.hpp>
#include <GLES2/gl2.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderText.h>
#include <GLRenderGeometry.h>
#include <GLRenderLine.h>


//#define DEBUG_POSITION

class HeightLadder {
private:
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    const GLRenderLine* mGLRenderLine;
    GLuint mGLLadderLinesB[1];
    GLuint mGLLadderTextB[1];
    struct{
        int MAX_N_CHARS=6;
        string s="0";
        int nVertices=0;
        GLuint GLTextBuffer[1];
        GLuint GLOutlineLinesB[1];
    } mMainGLString;

    float mX,mY,mZ,mWidth,mHeight;
    int mLadderLinesOffset=0,mNLadderLinesToDraw=0;
    int mLadderSOffset1=0,mNLadderSToDraw1=0;
    int mLadderSOffset2=0,mNLadderSToDraw2=0;
    glm::mat4x4 mLadderLinesTM;
    glm::mat4x4 mLadderStringsTM;
    const int N_LADDER_L=4*5;
    const int METERS_PER_4_LADDERS=20;
    const int CHARS_PER_LADDER=6;
    const int N_LADDER_S=5;
    float meter_in_gl_translation;
    float outline_quad_height;
    float ladder_text_height;
    int outline_stroke_pixels;
    int ladder_stroke_pixels;
    int mCurrentLadderStartValue;
    void updateLadderTextForRange(int startValue);
public:
    HeightLadder(const GLRenderColor* glRenderColor,const GLRenderLine* glRenderLine,const GLRenderText* glRenderText);
    void setWorldPosition(float x,float y,float z,float width,float height,int strokeW);
    void updateGL(float value);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    const float RATIO=3.0f/1.0f;
};
#endif
