
#ifndef HEIGHTLADDER
#define HEIGHTLADDER

#include <glm/mat4x4.hpp>
#include <GLES2/gl2.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLRenderText.h>
#include <GLRenderColor.h>


//#define DEBUG_POSITION

class HeightLadder {
private:
#ifdef DEBUG_POSITION
    GLuint mGLDebugB[1];
#endif
    const GLRenderText* mGLRenderText;
    const GLRenderColor* mGLRenderColor;
    GLuint mGLLadderLines_Arrow_B[1];
    GLuint mGLLadderTextB[1];
    //int ladderStringsO=0,nLadderStringsToDraw=0;
    struct ValueString{
        int maxChars=6;
        string s="0m";
        int nVertices=0;
        GLuint mGLBuffer[1];
    } mValueS;
    glm::mat4x4 mHeightTransM;
    glm::mat4x4 mLinesTM;
    glm::mat4x4 mLadderStringsTM;
    int mLinesOffset=0,mNLinesToDraw=0;
    int mLadderSOffset=0,mNLadderSToDraw=0;
    float meter_in_gl_translation;
    int mCurrentLadderStartValue;
    float text_height;
    float mX,mY,mZ,mWidth,mHeight;
    const int CHARS_PER_LADDER=6;
    const int N_LADDER_L=4*5;
    const int N_LADDER_S=5;
    const int METERS_PER_4Ladders=20;
    void updateLadderStringsForRange(int startVal);
    //void updateValueString(float value);
    float artMovC=0;
    bool artMovReverse= false;
public:
    HeightLadder(const GLRenderColor* glRenderColor,const GLRenderText* glRenderText);
    void setWorldPosition(float x,float y,float z,float width,float height,float strokeWidth);
    void updateGL(float value);
    void drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM);
    const float RATIO=3.0f/1.0f;
};

#endif
