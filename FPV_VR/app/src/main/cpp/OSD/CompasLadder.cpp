
#include "CompasLadder.h"
#include "../Helper/GeometryHelper.h"
#include "../Helper/StringHelper.h"
#include <sstream>
#include <cstdlib>

//#define ARTIFICIAL_MOVEMENT

CompasLadder::CompasLadder(const GLRenderColor *glRenderColor, const GLRenderText *glRenderText,bool eHomeArrow,bool invertHeading) {
    this->eHomeArrow=eHomeArrow;
    this->invertHeading=invertHeading;
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderColor=glRenderColor;
    mGLRenderText=glRenderText;
    glGenBuffers(1,mGLLines_HomeArrowB);
    glGenBuffers(1,mGLTextB1);
    glGenBuffers(1,mHeadingS.mGLBuffer);
    srand (static_cast <unsigned> (time(0)));
}

void CompasLadder::setWorldPosition(float x, float y, float z, float width, float height,float strokeW) {
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
    float home_arrow_width_height=height/3.0f;
    /*float long_lines_height=height*1.0f/3.0f;
    float short_lines_height=long_lines_height*0.5f;
    float extreme_short_lines_length=short_lines_height*0.2f;*/
    //make The lines & home Arrow & Arrow pointing to the middle
    float tmp[7*6*nLines+7*3*1+7*3*1];
    int c=0;
    for(int i=0;i<nLines;i+=4){
        makeColoredRect(tmp,c*7*6,glm::vec3(x+(0+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                                  glm::vec3(lines_width,0,0),glm::vec3(0,-extreme_short_lines_length,0),1,1,1,1);
        c++;
        makeColoredRect(tmp,c*7*6,glm::vec3(x+(1+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                                  glm::vec3(lines_width,0,0),glm::vec3(0,-long_lines_height,0),1,1,1,1);
        c++;
        makeColoredRect(tmp,c*7*6,glm::vec3(x+(2+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                                  glm::vec3(lines_width,0,0),glm::vec3(0,-short_lines_height,0),1,1,1,1);
        c++;
        makeColoredRect(tmp,c*7*6,glm::vec3(x+(3+i)*d_between_lines-lines_width/2.0f,y+mHeight/2.0f,z),
                                  glm::vec3(lines_width,0,0),glm::vec3(0,-long_lines_height,0),1,1,1,1);
        c++;
    }
    //Home Arrow
    makeColoredTriangle1(tmp,7*6*nLines,glm::vec3(x-home_arrow_width_height/2.0f,y+height,z),
                                  home_arrow_width_height,-home_arrow_width_height,0,1,0,1);
    //Middle pointing arrow
    float middle_arrow_width_height=mHeight/8.0f;
    makeColoredTriangle1(tmp,7*6*nLines+7*3,glm::vec3(x-middle_arrow_width_height/2.0f+mWidth/2.0f,y+mHeight/2.0f+middle_arrow_width_height,
                                                               z),middle_arrow_width_height,-middle_arrow_width_height,1,1,1,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLLines_HomeArrowB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    float tmp2[5*6*nChars];
    mGLRenderText->convertStringToVECs_UVs("S",x-mGLRenderText->getStringLength("N",text_height)/2.0f+d_between_lines*4*0,
                                           y,z,text_height,tmp2,5*6*0);
    mGLRenderText->convertStringToVECs_UVs("W",x-mGLRenderText->getStringLength("E",text_height)/2.0f+d_between_lines*4*1,
                                           y,z,text_height,tmp2,5*6*1);
    mGLRenderText->convertStringToVECs_UVs("N",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*2,
                                           y,z,text_height,tmp2,5*6*2);
    mGLRenderText->convertStringToVECs_UVs("E",x-mGLRenderText->getStringLength("W",text_height)/2.0f+d_between_lines*4*3,
                                           y,z,text_height,tmp2,5*6*3);
    mGLRenderText->convertStringToVECs_UVs("S",x-mGLRenderText->getStringLength("N",text_height)/2.0f+d_between_lines*4*4,
                                           y,z,text_height,tmp2,5*6*4);
    mGLRenderText->convertStringToVECs_UVs("W",x-mGLRenderText->getStringLength("E",text_height)/2.0f+d_between_lines*4*5,
                                           y,z,text_height,tmp2,5*6*5);
    mGLRenderText->convertStringToVECs_UVs("N",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*6,
                                           y,z,text_height,tmp2,5*6*6);
    mGLRenderText->convertStringToVECs_UVs("E",x-mGLRenderText->getStringLength("S",text_height)/2.0f+d_between_lines*4*7,
                                           y,z,text_height,tmp2,5*6*7);
    glBindBuffer(GL_ARRAY_BUFFER, mGLTextB1[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp2),
                 tmp2, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //360D
    float tmp3[5*6*mHeadingS.maxChars];
    mHeadingS.s="";
    float sLength=mGLRenderText->getStringLength(mHeadingS.s,height/3.0f);
    mHeadingS.nVertices=mGLRenderText->convertStringToVECs_UVs(mHeadingS.s,x+width/2.0f-sLength/2.0f,y+height*2.0f/3.0f,z,
                                                              height/3.0f,tmp2,0);
    glBindBuffer(GL_ARRAY_BUFFER, mHeadingS.mGLBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp3),
                 tmp3, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CompasLadder::updateGL(float headingD,float headingHomeD) {
#ifdef ARTIFICIAL_MOVEMENT
    /*if(artMovC>=180){
        artMovReverse=true;
    }
    if(artMovC<=0){
        artMovReverse=false;
    }
    float random1 = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    random1*=0.5f;
    if(artMovReverse){
        artMovC-=random1;
    }else{
        artMovC+=random1;
    }
    headingD=artMovC;*/
    float random1 = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    if(random1>0.99){
        artMovReverse=!artMovReverse;
    }
    if(artMovReverse){
        artMovC-=0.5f;
    }else{
        artMovC+=0.5f;
    }
    headingD=artMovC;

#endif
    if(invertHeading){
        headingD*=-1;
    }
    while(headingD>=360){
        headingD-=360;
    }
    //headingD=std::fmod(headingD,360.0f);
    while(headingHomeD>=360){
        headingHomeD=0;
    }
    while(headingD<0){
        headingD+=360;
    }
    while(headingHomeD<0){
        headingHomeD+=360;
    }
    float heading1=headingD;
    charOffset=0;
    while(heading1>0){
        heading1-=90;
        charOffset+=1;
    }
    heading1=headingD;
    linesOffset=0;
    while(heading1>0){
        heading1-=22.5f;
        linesOffset+=1;
    }
    nCharsToDraw=4;
    nLinesToDraw=16;
    mCompasTM = glm::translate(glm::mat4(1.0f),glm::vec3(-degree_in_gl_translation*headingD,0,0));
    if(eHomeArrow){
        float homeTransl=(headingHomeD+180)-headingD;
        if(homeTransl<0){
            homeTransl+=360;
        }
        mHomeArrowTM = glm::translate(glm::mat4(1.0f), glm::vec3(degree_in_gl_translation*homeTransl,0,0));
    }
    mHeadingS.s=intToString((int)headingD,mHeadingS.maxChars-1);
    mHeadingS.s.append("D");//"°"
    float sLength=mGLRenderText->getStringLength(mHeadingS.s,mHeadingS.height);
    float tmp[5*6*mHeadingS.maxChars];
    mHeadingS.nVertices=mGLRenderText->convertStringToVECs_UVs(mHeadingS.s,mX+mWidth*0.5f-sLength*0.5f,mY+mHeight-mHeadingS.height,mZ,
                                                               mHeadingS.height,tmp,0);
    glBindBuffer(GL_ARRAY_BUFFER, mHeadingS.mGLBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                 tmp, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CompasLadder::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM,ProjM,0,3*2);
    mGLRenderColor->afterDraw();
#endif
    glm::mat4x4 MVM=mCompasTM*ViewM;
    mGLRenderColor->beforeDraw(mGLLines_HomeArrowB[0]);
    //Render all the lines
    mGLRenderColor->draw(MVM,ProjM,3*2*linesOffset,3*2*nLinesToDraw);
    if(eHomeArrow){
        //Render the home arrow
        mGLRenderColor->draw(mHomeArrowTM*ViewM,ProjM,3*2*nLines,3);
    }
    //Render the point to the middle arrow
    mGLRenderColor->draw(ViewM,ProjM,3*2*nLines+3,3);
    mGLRenderColor->afterDraw();

    mGLRenderText->beforeDraw(mGLTextB1[0]);
    //Render the N W S E chars
    mGLRenderText->draw(MVM,ProjM,3*2*charOffset,3*2*nCharsToDraw,1,1,1);
    mGLRenderText->afterDraw();

    mGLRenderText->beforeDraw(mHeadingS.mGLBuffer[0]);
    //Render the 360° (heading) chars
    mGLRenderText->draw(ViewM,ProjM,0,mHeadingS.nVertices,1,1,1);
    mGLRenderText->afterDraw();
}

/*float TinyGPS::distance_between (float lat1, float long1, float lat2, float long2)
{
    // returns distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude. Uses great-circle
    // distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    float delta = radians(long1-long2);
    float sdlong = sin(delta);
    float cdlong = cos(delta);
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    float slat1 = sin(lat1);
    float clat1 = cos(lat1);
    float slat2 = sin(lat2);
    float clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = sq(delta);
    delta += sq(clat2 * sdlong);
    delta = sqrt(delta);
    float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    return delta * 6372795;
}

/*float TinyGPS::course_to (float lat1, float long1, float lat2, float long2)
{
    // returns course in degrees (North=0, West=270) from position 1 to position 2,
    // both specified as signed decimal-degrees latitude and longitude.
    // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
    // Courtesy of Maarten Lamers
    float dlon = radians(long2-long1);
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    float a1 = sin(dlon) * cos(lat2);
    float a2 = sin(lat1) * cos(lat2) * cos(dlon);
    a2 = cos(lat1) * sin(lat2) - a2;
    a2 = atan2(a1, a2);
    if (a2 < 0.0)
    {
        a2 += TWO_PI;
    }
    return degrees(a2);
}*/

