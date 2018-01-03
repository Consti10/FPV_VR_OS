
#include <cstdlib>
#include "HeightLadder.h"
#include "../Helper/GeometryHelper.h"
#include "../Helper/StringHelper.h"

//#define ARTIFICIAL_MOVEMENT

HeightLadder::HeightLadder(const GLRenderColor *glRenderColor, const GLRenderText *glRenderText) {
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderColor=glRenderColor;
    mGLRenderText=glRenderText;
    glGenBuffers(1,mGLLadderLines_Arrow_B);
    glGenBuffers(1,mGLLadderTextB);
    glGenBuffers(1,mValueS.mGLBuffer);
    srand (static_cast <unsigned> (time(0)));
}
void HeightLadder::setWorldPosition(float x, float y, float z, float width, float height,float strokeWidth) {
#ifdef DEBUG_POSITION
    float debug[7*6];
    makeColoredRect(debug,0,glm::vec3(x,y,z),glm::vec3(width,0,0),glm::vec3(0,height,0),0,0,1,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug),
                 debug, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    mX=x;
    mY=y;
    mZ=z;
    mWidth=width;
    mHeight=height;
    float lines_width=width/3.0f;
    float lines_height=strokeWidth;
    float d_between_lines=height/16.0f;
    meter_in_gl_translation=mHeight/(4*METERS_PER_4Ladders);
    text_height=0.2f;
    //Setup The Ladder lines (needs only to be done one time)
    float tmpLinesB[7*6*N_LADDER_L];
    int c=0;
    for(int i=0;i<N_LADDER_L;i+=4){
        makeColoredRect(tmpLinesB,c*7*6,glm::vec3(x+width*2.0f/3.0f,y+c*d_between_lines+lines_height/2.0f,z),
                                  glm::vec3(lines_width,0,0), glm::vec3(0,lines_height,0),1,1,1,1);
        c++;
        makeColoredRect(tmpLinesB,c*7*6,glm::vec3(x+width*2.0f/3.0f+lines_width*0.5f,y+c*d_between_lines+lines_height/2.0f,z),
                                  glm::vec3(lines_width*0.5f,0,0), glm::vec3(0,lines_height,0),1,1,1,1);
        c++;
        makeColoredRect(tmpLinesB,c*7*6,glm::vec3(x+width*2.0f/3.0f+lines_width*0.5f,y+c*d_between_lines+lines_height/2.0f,z),
                                  glm::vec3(lines_width*0.5f,0,0), glm::vec3(0,lines_height,0),1,1,1,1);
        c++;
        makeColoredRect(tmpLinesB,c*7*6,glm::vec3(x+width*2.0f/3.0f+lines_width*0.5f,y+c*d_between_lines+lines_height/2.0f,z),
                                  glm::vec3(lines_width*0.5f,0,0), glm::vec3(0,lines_height,0),1,1,1,1);
        c++;
    }
    float tmpIndicatorArrowB[7*3];
    float arrowW=0.1f;
    float arrowH=0.1f;
    makeColoredTriangle3(tmpIndicatorArrowB,0,glm::vec3(x+mWidth,y+height/2.0f,z),glm::vec3(arrowW,arrowH/2.0f,0.0f),glm::vec3(arrowW,-arrowH/2.0f,0.0f),1.0f,1.0f,1.0f,1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderLines_Arrow_B[0]);
    /*glBufferData(GL_ARRAY_BUFFER, sizeof(tmpLinesB)+sizeof(tmpIndicatorArrowB),
                 tmpLinesB, GL_STATIC_DRAW);*/
    glBufferData(GL_ARRAY_BUFFER,sizeof(tmpLinesB)+sizeof(tmpIndicatorArrowB),NULL,GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(tmpLinesB),tmpLinesB);
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(tmpLinesB),sizeof(tmpIndicatorArrowB),tmpIndicatorArrowB);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //updateValueString(0);
    mCurrentLadderStartValue=0;
    updateLadderStringsForRange(0);

}

void HeightLadder::updateGL(float value) {
#ifdef ARTIFICIAL_MOVEMENT
    if(artMovC>=300){
        artMovReverse=true;
    }
    if(artMovC<=0){
        artMovReverse=false;
    }
    if(artMovReverse){
        artMovC-=0.5f;
    }else{
        artMovC+=0.5f;
    }
    value=artMovC;
#endif
    if(value>99000){
        //values higher than 99 thousand meters are not supported.
        value=99000;
    }
    //Update the string containing the current height as a number
    //updateValueString(value);
    //calculate the offsets/translation values
    //1) For the Lines
    float linesT=0;
    if(value<METERS_PER_4Ladders*2){
        //value range: 0..40m
        float t;
        mLinesOffset=0;
        mNLinesToDraw=8;
        t=value;
        while(t>0.0f){
            t-=((float)METERS_PER_4Ladders)*0.25f;
            mNLinesToDraw++;
        }
        t=value;
        linesT=t-METERS_PER_4Ladders*2;
    }else{
        //value range: 40...X
        float t;
        mNLinesToDraw=16;
        mLinesOffset=0;
        t=value;
        while(t>METERS_PER_4Ladders){
            t-=METERS_PER_4Ladders;
        }
        linesT=t;
        while(t>0){
            t-=METERS_PER_4Ladders*0.25f;
            mLinesOffset++;
        }
    }
    mLinesTM = glm::translate(glm::mat4(1.0f),glm::vec3(0,-meter_in_gl_translation*linesT,0));
    //2) for the ladder strings
    float ladderStringsT=0;
    if(value<METERS_PER_4Ladders*2){
        //value range: 0..40m
        if(mCurrentLadderStartValue!=0){
            updateLadderStringsForRange(0);
            mCurrentLadderStartValue=0;
        }
        mLadderSOffset=0;
        mNLadderSToDraw=2;
        float t=value;
        while(t>0.0f){
            t-=METERS_PER_4Ladders;
            mNLadderSToDraw++;
        }
        ladderStringsT=value-METERS_PER_4Ladders*2;
    }else{
        //value range: 40m...Xm
        mLadderSOffset=0;
        mNLadderSToDraw=4;
        float t=value;
        int i=0;
        while(t>METERS_PER_4Ladders){
            t-=METERS_PER_4Ladders;
            i++;
        }
        ladderStringsT=t;
        if(t>0){
            mLadderSOffset++;
        }
        int ladderStartValue=i*METERS_PER_4Ladders-2*METERS_PER_4Ladders;
        if(mCurrentLadderStartValue!=ladderStartValue){
            updateLadderStringsForRange(ladderStartValue);
            mCurrentLadderStartValue=ladderStartValue;
        }
    }
    mLadderStringsTM = glm::translate(glm::mat4(1.0f),glm::vec3(0,-meter_in_gl_translation*ladderStringsT,0));
}

void HeightLadder::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM,ProjM,0,3*2);
    mGLRenderColor->afterDraw();
#endif
    mGLRenderColor->beforeDraw(mGLLadderLines_Arrow_B[0]);
    mGLRenderColor->draw(mLinesTM*ViewM,ProjM,2*3*mLinesOffset,2*3*mNLinesToDraw);
    //Height Indicator arrow
    mGLRenderColor->draw(ViewM,ProjM,2*3*N_LADDER_L,3);
    mGLRenderColor->afterDraw();

    mGLRenderText->beforeDraw(mGLLadderTextB[0]);
    mGLRenderText->draw(mLadderStringsTM*ViewM,ProjM,2*3*CHARS_PER_LADDER*mLadderSOffset,2*3*CHARS_PER_LADDER*mNLadderSToDraw,1,1,1);
    mGLRenderText->afterDraw();

    /*mGLRenderText->beforeDraw(mValueS.mGLBuffer[0]);
    mGLRenderText->draw(ViewM,ProjM,0,mValueS.nVertices,1,1,1);
    mGLRenderText->afterDraw();*/
}
/*void HeightLadder::updateValueString(float value) {
    //Set up the Value String. This data changes every time update() is called
    float tmpValueSB[5*6*mValueS.maxChars];
    memset(tmpValueSB, 0, sizeof(tmpValueSB));
    mValueS.s=intToString((int)value,mValueS.maxChars-1);
    mValueS.s.append("m");
    mValueS.nVertices=mGLRenderText->convertStringToVECs_UVs(mValueS.s,mX+1,mY+mHeight*0.5f-text_height*0.5f,mZ,text_height,tmpValueSB,0);
    glBindBuffer(GL_ARRAY_BUFFER, mValueS.mGLBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmpValueSB),
                 tmpValueSB, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}*/

void HeightLadder::updateLadderStringsForRange(int startVal) {
    float tmp2[5*6*CHARS_PER_LADDER*N_LADDER_S];
    for(int i=0;i<5*6*CHARS_PER_LADDER*N_LADDER_S;i++){
        tmp2[i]=0.0f;
    }
    for(int i=0;i<N_LADDER_S;i++){
        string s=intToString(i*METERS_PER_4Ladders+startVal,CHARS_PER_LADDER-1);
        s.append("m");
        float length=mGLRenderText->getStringLength(s,0.2f);
        mGLRenderText->convertStringToVECs_UVs(s,mX+mWidth-length-mWidth/3.0f,mY+mHeight/4.0f*i-text_height/2.0f,mZ,text_height,
                                               tmp2,i*5*6*CHARS_PER_LADDER);
    }
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderTextB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp2),
                 tmp2, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


