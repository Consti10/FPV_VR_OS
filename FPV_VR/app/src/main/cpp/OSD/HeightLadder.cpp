
#include <cstdlib>
#include "HeightLadder.h"
#include "../Helper/GeometryHelper.h"
#include "../Helper/StringHelper.h"

//#define ARTIFICIAL_MOVEMENT

HeightLadder::HeightLadder(const GLRenderColor *glRenderColor,const GLRenderLine* glRenderLine, const GLRenderText *glRenderText) {
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderColor=glRenderColor;
    mGLRenderLine=glRenderLine;
    mGLRenderText=glRenderText;
    glGenBuffers(1,mGLLadderLinesB);
    glGenBuffers(1,mMainGLString.GLOutlineLinesB);
    glGenBuffers(1,mMainGLString.GLTextBuffer);
    glGenBuffers(1,mGLLadderTextB);
}

void HeightLadder::setWorldPosition(float x, float y, float z, float width, float height,int strokeW) {
#ifdef DEBUG_POSITION
    float debug[7 * 6];
    makeColoredRect(debug, 0, glm::vec3(x, y, z), glm::vec3(width, 0, 0), glm::vec3(0, height, 0),
                    0, 0, 1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug),
                 debug, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    mX = x;
    mY = y;
    mZ = z;
    mWidth = width;
    mHeight = height;
    outline_stroke_pixels=strokeW;
    ladder_stroke_pixels=strokeW;
    meter_in_gl_translation=mHeight/(4*METERS_PER_4_LADDERS);
    //2/3 of the width are for the fonts, 1/3 of the width is for the ladder lines
    //create the outline for the main string.
    //quad, made of 4 lines
    float outline_quad_width=mWidth*2.0f/3.0f;
    outline_quad_height=outline_quad_width*2.0f/3.0f;
    float tmp[7*2*4];
    //left and right
    makeColoredLine(tmp,7*2*0,glm::vec3(mX,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ),glm::vec3(mX,mY+mHeight/2.0f+outline_quad_height/2.0f,mZ),1,1,1,1);
    makeColoredLine(tmp,7*2*1,glm::vec3(mX+outline_quad_width,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ),glm::vec3(mX+outline_quad_width,mY+mHeight/2.0f+outline_quad_height/2.0f,mZ),1,1,1,1);
    //top and bottom
    makeColoredLine(tmp,7*2*2,glm::vec3(mX,mY+mHeight/2.0f+outline_quad_height/2.0f,mZ),glm::vec3(mX+outline_quad_width,mY+mHeight/2.0f+outline_quad_height/2.0f,mZ),1,1,1,1);
    makeColoredLine(tmp,7*2*3,glm::vec3(mX,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ),glm::vec3(mX+outline_quad_width,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ),1,1,1,1);
    glBindBuffer(GL_ARRAY_BUFFER, mMainGLString.GLOutlineLinesB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Setup the ladder lines
    float d_between_lines=mHeight/16.0f;
    float ladder_lines_width=mWidth/3.0f;
    float tmpLinesB[7*2*N_LADDER_L];
    int c=0;
    for(int i=0;i<N_LADDER_L;i+=4){
        makeColoredLine(tmpLinesB,c*7*2,glm::vec3(x+mWidth,y+c*d_between_lines,z),
                        glm::vec3(x+mWidth-ladder_lines_width,y+c*d_between_lines,z),1,1,1,1);
        c++;
        makeColoredLine(tmpLinesB,c*7*2,glm::vec3(x+mWidth,y+c*d_between_lines,z),
                        glm::vec3(x+mWidth-ladder_lines_width/2.0f,y+c*d_between_lines,z),1,1,1,1);
        c++;
        makeColoredLine(tmpLinesB,c*7*2,glm::vec3(x+mWidth,y+c*d_between_lines,z),
                        glm::vec3(x+mWidth-ladder_lines_width/2.0f,y+c*d_between_lines,z),1,1,1,1);
        c++;
        makeColoredLine(tmpLinesB,c*7*2,glm::vec3(x+mWidth,y+c*d_between_lines,z),
                        glm::vec3(x+mWidth-ladder_lines_width/2.0f,y+c*d_between_lines,z),1,1,1,1);
        c++;
    }
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderLinesB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmpLinesB),
                 tmpLinesB, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    mLadderLinesOffset=0;
    mNLadderLinesToDraw=16;


    //initialize the main string with "0"
    float tmp2[5*6*mMainGLString.MAX_N_CHARS];
    mMainGLString.nVertices=mGLRenderText->convertStringToVECs_UVs(mMainGLString.s,mX,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ,outline_quad_height,tmp2,0);
    glBindBuffer(GL_ARRAY_BUFFER,mMainGLString.GLTextBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp2),
                 tmp2, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //initialize the ladder strings
    mCurrentLadderStartValue=0;
    ladder_text_height=outline_quad_height*0.5f;
    updateLadderTextForRange(mCurrentLadderStartValue);
}
void HeightLadder::updateLadderTextForRange(int startValue) {
    float tmp[5*6*CHARS_PER_LADDER*N_LADDER_S];
    memset (&tmp, 0, sizeof(tmp));
    for(int i=0;i<N_LADDER_S;i++){
        string s=intToString(i*METERS_PER_4_LADDERS+startValue,CHARS_PER_LADDER-1);
        s.append("m");
        float length=mGLRenderText->getStringLength(s,ladder_text_height);
        mGLRenderText->convertStringToVECs_UVs(s,mX+mWidth*2.0f/3.0f-length,mY+mHeight/4.0f*i-ladder_text_height/2.0f,mZ,ladder_text_height,
                                               tmp,i*5*6*CHARS_PER_LADDER);
    }
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderTextB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//float c=0;
void HeightLadder::updateGL(float value) {
    //c+=0.1;
    //value=c;
//update the main string that holds the accurate height
    mMainGLString.s=intToString((int)round(value),mMainGLString.MAX_N_CHARS);
    float tmp2[5*6*mMainGLString.MAX_N_CHARS];
    mMainGLString.nVertices=mGLRenderText->convertStringToVECs_UVs(mMainGLString.s,mX,mY+mHeight/2.0f-outline_quad_height/2.0f,mZ,outline_quad_height,tmp2,0);
    glBindBuffer(GL_ARRAY_BUFFER,mMainGLString.GLTextBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp2),
                 tmp2, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
//values higher than 99 thousand meters are not supported. Also negative values cannot be represented by the ladder lines
    if(value>99000){
        value=99000;
    }
    if(value<0){
        value=0;
    }
//1) For the ladder lines
    float linesT=0;
    if(value<METERS_PER_4_LADDERS*2){
        //value range: 0..40m
        float t;
        mLadderLinesOffset=0;
        mNLadderLinesToDraw=8;
        t=value;
        while(t>0.0f){
            t-=((float)METERS_PER_4_LADDERS)*0.25f;
            mNLadderLinesToDraw++;
        }
        t=value;
        linesT=t-METERS_PER_4_LADDERS*2;
    }else{
        //value range: 40...X
        float t;
        mNLadderLinesToDraw=16;
        mLadderLinesOffset=0;
        t=value;
        while(t>METERS_PER_4_LADDERS){
            t-=METERS_PER_4_LADDERS;
        }
        linesT=t;
        while(t>0){
            t-=METERS_PER_4_LADDERS*0.25f;
            mLadderLinesOffset++;
        }
    }
    mLadderLinesTM = glm::translate(glm::mat4(1.0f),glm::vec3(0,-meter_in_gl_translation*linesT,0));
//2) for the ladder strings
    int ladderSOffset;
    int nLadderSToDraw;
    float ladderStringsT=0;
    if(value<METERS_PER_4_LADDERS*2){
        //value range: 0..40m
        if(mCurrentLadderStartValue!=0){
            updateLadderTextForRange(0);
            mCurrentLadderStartValue=0;
        }
        ladderSOffset=0;
        nLadderSToDraw=2;
        float t=value;
        while(t>0.0f){
            t-=METERS_PER_4_LADDERS;
            nLadderSToDraw++;
        }
        ladderStringsT=value-METERS_PER_4_LADDERS*2;
    }else{
        //value range: 40m...Xm
        ladderSOffset=0;
        nLadderSToDraw=4;
        float t=value;
        int i=0;
        while(t>METERS_PER_4_LADDERS){
            t-=METERS_PER_4_LADDERS;
            i++;
        }
        ladderStringsT=t;
        if(t>0){
            ladderSOffset++;
        }
        int ladderStartValue=i*METERS_PER_4_LADDERS-2*METERS_PER_4_LADDERS;
        if(mCurrentLadderStartValue!=ladderStartValue){
            updateLadderTextForRange(ladderStartValue);
            mCurrentLadderStartValue=ladderStartValue;
        }
    }
    float translation=-meter_in_gl_translation*ladderStringsT;
    mLadderStringsTM = glm::translate(glm::mat4(1.0f),glm::vec3(0,translation,0));
//we have to disable up to 1 ladder string when occluded by the outlines
    for(int i=0;i<nLadderSToDraw;i++){
        float inLadderCoords=translation+((i+ladderSOffset)*meter_in_gl_translation*METERS_PER_4_LADDERS);
        //LOGV3("HEIGHT:%f: height:%f, i:%d, inCoord:%f",mHeight,value,i,inLadderCoords);
        float occludeMax=mHeight/2.0f+outline_quad_height/2.0f+ladder_text_height/2.0f;
        float occludeMin=mHeight/2.0f-outline_quad_height/2.0f-ladder_text_height/2.0f;
        if(inLadderCoords>occludeMin && inLadderCoords<occludeMax) {
            //this ladder string is occluded
            mLadderSOffset1=ladderSOffset;
            mNLadderSToDraw1=i;
            //LOGV3("%f",inLadderCoords);
            if(nLadderSToDraw>i+1){
                mLadderSOffset2=mLadderSOffset1+i+1;
                mNLadderSToDraw2=nLadderSToDraw-i-1;
            }
            break;
        }
        mLadderSOffset1=ladderSOffset;
        mNLadderSToDraw1=nLadderSToDraw;
    }
    //mLadderSOffset1=ladderSOffset;
    //mNLadderSToDraw1=nLadderSToDraw;
    //mLadderSOffset1=0;
    //mNLadderSToDraw1=5;
}

void HeightLadder::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM, ProjM, 0, 3 * 2);
    mGLRenderColor->afterDraw();
#endif

    //draw the main string chars
    mGLRenderText->beforeDraw(mMainGLString.GLTextBuffer[0]);
    mGLRenderText->draw(ViewM,ProjM,0,mMainGLString.nVertices,1,1,1);
    mGLRenderText->afterDraw();

    //draw the main string outline lines (4 lines total)
    mGLRenderLine->beforeDraw(mMainGLString.GLOutlineLinesB[0]);
    mGLRenderLine->draw(ViewM, ProjM, 0,2*4,outline_stroke_pixels);
    mGLRenderLine->afterDraw();

    //draw the ladder lines
    mGLRenderLine->beforeDraw(mGLLadderLinesB[0]);
    mGLRenderLine->draw(mLadderLinesTM*ViewM, ProjM, 2*mLadderLinesOffset,2*mNLadderLinesToDraw,ladder_stroke_pixels);
    mGLRenderLine->afterDraw();

    //draw the ladder strings
    mGLRenderText->beforeDraw(mGLLadderTextB[0]);
    //mGLRenderText->draw(mLadderStringsTM*ViewM,ProjM,2*3*CHARS_PER_LADDER*mLadderSOffset,2*3*CHARS_PER_LADDER*mNLadderSToDraw,1,1,1);
    mGLRenderText->draw(mLadderStringsTM*ViewM,ProjM,2*3*CHARS_PER_LADDER*mLadderSOffset1,2*3*CHARS_PER_LADDER*mNLadderSToDraw1,1,1,1);
    mGLRenderText->draw(mLadderStringsTM*ViewM,ProjM,2*3*CHARS_PER_LADDER*mLadderSOffset2,2*3*CHARS_PER_LADDER*mNLadderSToDraw2,1,1,1);
    mGLRenderText->afterDraw();
}