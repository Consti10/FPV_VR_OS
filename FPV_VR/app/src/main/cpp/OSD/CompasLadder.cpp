
#include "CompasLadder.h"
#include "../Helper/GeometryHelper.h"
#include "../Helper/StringHelper.h"
#include <sstream>
#include <cstdlib>

//#define ARTIFICIAL_MOVEMENT

CompasLadder::CompasLadder(const GLRenderColor *glRenderColor,const GLRenderLine* glRenderLine, const GLRenderText *glRenderText,bool eHomeArrow,bool invertHeading) {
    this->eHomeArrow=eHomeArrow;
    this->invertHeading=invertHeading;
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderColor=glRenderColor;
    mGLRenderText=glRenderText;
    mGLRenderLine=glRenderLine;
    glGenBuffers(1,mGLLadderLinesB);
    glGenBuffers(1,mGLLadderTextB);
    glGenBuffers(1,mGLHomeArrowB);
    glGenBuffers(1,mHeadingS.mGLTextB);
    glGenBuffers(1,mHeadingS.mGLOutlineLinesB);
}

void CompasLadder::setWorldPosition(float x, float y, float z, float width, float height,int strokeW) {
#ifdef DEBUG_POSITION
    float debug[7*6];
    makeColoredRect(debug,0,glm::vec3(x,y,z),glm::vec3(width,0,0),glm::vec3(0,height,0),1,0,0,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug),
                 debug, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    mWidth=width;
    mHeight=height;
    mX=x;
    mY=y;
    mZ=z;
    outline_stroke_pixels=strokeW;
    ladder_stroke_pixels=strokeW;
    degree_in_gl_translation=width/360.0f;
    float d_between_lines=width/16.0f;
    float lines_width=strokeW;
    float text_height=height*0.5f;
    mHeadingS.height=text_height;
    /*float long_lines_height=text_height*0.8f;
    float short_lines_height=long_lines_height*2.0f/3.0f;*/
    float long_lines_height=text_height*0.5f;
    float short_lines_height=long_lines_height;
    float extreme_short_lines_length=short_lines_height*0.2f;
//make the ladder lines
    float tmp[7*2*N_LINES];
    int c=0;
    for(int i=0;i<N_LINES;i+=4){
        makeColoredLine(tmp,c*7*2,glm::vec3(x+(0+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                        glm::vec3(x+(0+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f-extreme_short_lines_length,z),1,1,1,1);
        c++;
        makeColoredLine(tmp,c*7*2,glm::vec3(x+(1+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                        glm::vec3(x+(1+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f-long_lines_height,z),1,1,1,1);
        c++;
        makeColoredLine(tmp,c*7*2,glm::vec3(x+(2+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                        glm::vec3(x+(2+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f-short_lines_height,z),1,1,1,1);
        c++;
        makeColoredLine(tmp,c*7*2,glm::vec3(x+(3+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                        glm::vec3(x+(3+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f-long_lines_height,z),1,1,1,1);
        c++;
    }
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderLinesB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

//create the outline for the main string.
    //quad, made of 4 lines
    outline_quad_height=mHeight/2.0f;
    outline_quad_width=outline_quad_height*2;
    float tmp2[7*2*4];
    //left and right
    makeColoredLine(tmp2,7*2*0,glm::vec3(mX+mWidth/2.0f-outline_quad_width/2.0f,mY+mHeight,mZ),glm::vec3(mX+mWidth/2.0f-outline_quad_width/2.0f,mY+mHeight-outline_quad_height,mZ),1,1,1,1);
    makeColoredLine(tmp2,7*2*1,glm::vec3(mX+mWidth/2.0f+outline_quad_width/2.0f,mY+mHeight,mZ),glm::vec3(mX+mWidth/2.0f+outline_quad_width/2.0f,mY+mHeight-outline_quad_height,mZ),1,1,1,1);
    //top and bottom
    makeColoredLine(tmp2,7*2*2,glm::vec3(mX+mWidth/2.0f-outline_quad_width/2.0f,mY+mHeight,mZ),glm::vec3(mX+mWidth/2.0f+outline_quad_width/2.0f,mY+mHeight,mZ),1,1,1,1);
    makeColoredLine(tmp2,7*2*3,glm::vec3(mX+mWidth/2.0f-outline_quad_width/2.0f,mY+mHeight-outline_quad_height,mZ),glm::vec3(mX+mWidth/2.0f+outline_quad_width/2.0f,mY+mHeight-outline_quad_height,mZ),1,1,1,1);
    glBindBuffer(GL_ARRAY_BUFFER, mHeadingS.mGLOutlineLinesB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp2),
                 tmp2, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
//create the S W N E chars
    float tmp3[5*6*nChars];
    mGLRenderText->convertStringToVECs_UVs("S",x-mGLRenderText->getStringLength("N",text_height)/2.0f+d_between_lines*4*0,
                                           y,z,text_height,tmp3,5*6*0);
    mGLRenderText->convertStringToVECs_UVs("W",x-mGLRenderText->getStringLength("E",text_height)/2.0f+d_between_lines*4*1,
                                           y,z,text_height,tmp3,5*6*1);
    mGLRenderText->convertStringToVECs_UVs("N",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*2,
                                           y,z,text_height,tmp3,5*6*2);
    mGLRenderText->convertStringToVECs_UVs("E",x-mGLRenderText->getStringLength("W",text_height)/2.0f+d_between_lines*4*3,
                                           y,z,text_height,tmp3,5*6*3);
    mGLRenderText->convertStringToVECs_UVs("S",x-mGLRenderText->getStringLength("N",text_height)/2.0f+d_between_lines*4*4,
                                           y,z,text_height,tmp3,5*6*4);
    mGLRenderText->convertStringToVECs_UVs("W",x-mGLRenderText->getStringLength("E",text_height)/2.0f+d_between_lines*4*5,
                                           y,z,text_height,tmp3,5*6*5);
    mGLRenderText->convertStringToVECs_UVs("N",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*6,
                                           y,z,text_height,tmp3,5*6*6);
    mGLRenderText->convertStringToVECs_UVs("E",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*7,
                                           y,z,text_height,tmp3,5*6*7);
    glBindBuffer(GL_ARRAY_BUFFER, mGLLadderTextB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp3),
                 tmp3, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    updateHeadingString(360);
//create the home arrow triangle
    float tmpHA[7*3];
    float home_arrow_width_height=height/3.0f;
    makeColoredTriangle1(tmpHA,0,glm::vec3(mX-home_arrow_width_height,mY-home_arrow_width_height*0.8f,mZ),
                         home_arrow_width_height,home_arrow_width_height,0,1,0,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLHomeArrowB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmpHA),
                 tmpHA, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CompasLadder::updateHeadingString(float heading) {
    //360D
    float tmp[5*6*mHeadingS.maxChars];
    mHeadingS.s=intToString((int)round(heading),4);
    mHeadingS.s.append("D");
    float sLength=mGLRenderText->getStringLength(mHeadingS.s,outline_quad_height);
    if(sLength>outline_quad_width){
        sLength=outline_quad_width;
    }
    mHeadingS.nVertices=mGLRenderText->convertStringToVECs_UVs(mHeadingS.s,mX+mWidth/2.0f-sLength/2.0f,mY+mHeight/2.0f,mZ,outline_quad_height,tmp,0);
    glBindBuffer(GL_ARRAY_BUFFER, mHeadingS.mGLTextB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//float c=0;
void CompasLadder::updateGL(float headingD,float headingHomeD) {
    //c+=0.01f;
    //headingD=c;
    if(invertHeading){
        headingD*=-1;
    }
    while(headingD>=360){
        headingD-=360;
    }
    while(headingHomeD>=360){
        headingHomeD-=360;
    }
    while(headingD<0){
        headingD+=360;
    }
    while(headingHomeD<0){
        headingHomeD+=360;
    }
    updateHeadingString(headingD);
    linesOffset=0;
    float tmpHeading=headingD;
    while(tmpHeading>0){
        tmpHeading-=22.5f;
        linesOffset+=1;
    }
    nLinesToDraw=16;
    charOffset=0;
    tmpHeading=headingD;
    while(tmpHeading>0){
        tmpHeading-=90;
        charOffset+=1;
    }
    nCharsToDraw=4;
    mHeadingTranslM = glm::translate(glm::mat4(1.0f),glm::vec3(-degree_in_gl_translation*headingD,0,0));
    if(eHomeArrow){
        float homeTransl=(headingHomeD+180)-headingD;
        while(homeTransl<0){
            homeTransl+=360;
        }
        mHomeArrowTM = glm::translate(glm::mat4(1.0f), glm::vec3(degree_in_gl_translation*homeTransl,0,0));
    }
}

void CompasLadder::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM,ProjM,0,3*2);
    mGLRenderColor->afterDraw();
#endif
    //Render the 360° (heading) chars
    mGLRenderText->beforeDraw(mHeadingS.mGLTextB[0]);
    mGLRenderText->draw(ViewM,ProjM,0,mHeadingS.nVertices,1,1,1);
    mGLRenderText->afterDraw();

    //draw the heading string outline lines (4 lines total)
    mGLRenderLine->beforeDraw(mHeadingS.mGLOutlineLinesB[0]);
    mGLRenderLine->draw(ViewM, ProjM, 0,2*4,outline_stroke_pixels);
    mGLRenderLine->afterDraw();

    //Render the N W S E chars
    mGLRenderText->beforeDraw(mGLLadderTextB[0]);
    mGLRenderText->draw(mHeadingTranslM*ViewM,ProjM,6*charOffset,6*nCharsToDraw,1,1,1);
    mGLRenderText->afterDraw();

    //Render all the lines
    mGLRenderLine->beforeDraw(mGLLadderLinesB[0]);
    mGLRenderLine->draw(mHeadingTranslM*ViewM,ProjM,2*linesOffset,2*nLinesToDraw,ladder_stroke_pixels);
    mGLRenderLine->afterDraw();

    //Render the home arrow triangle
    if(eHomeArrow){
        mGLRenderColor->beforeDraw(mGLHomeArrowB[0]);
        mGLRenderColor->draw(mHomeArrowTM*ViewM,ProjM,0,3);
        mGLRenderColor->afterDraw();
    }
}

