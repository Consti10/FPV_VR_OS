
#include <cstdlib>
#include <time.h>
#include "ModelRollPitch.h"

//#define ARTIFICIAL_MOVEMENT

ModelRollPitch::ModelRollPitch(const GLRenderColor* glRenderColor,bool roll,bool pitch,bool invRoll,bool invPitch) {
    mGLRenderColor=glRenderColor;
    this->roll=roll;
    this->pitch=pitch;
    this->invRoll=invRoll;
    this->invPitch=invPitch;
    glGenBuffers(1, GLBuffer);
    srand (static_cast <unsigned> (time(0)));
}
void ModelRollPitch::setWorldPosition(float x,float y,float z,float modelWidth){
    mX=x;
    mY=y;
    mZ=z;
    mModelWidth=modelWidth;
    float hW=mModelWidth/2.0f;
    float sixtW=mModelWidth/6.0f;
    float modelData[]= {
            //Kopter(gleichschenkliges Dreieck und 4 weitere)
            0.0f,0.0f,0.0f-sixtW,                // top
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            0.0f+sixtW,0.0f,0.0f+sixtW,                // bottom right
            0.0f, 0.0f, 1.0f, 1.0f,  //Color blue
            0.0f-sixtW,0.0f,0.0f+sixtW,                // bottom left
            1.0f, 1.0f, 0.0f, 1.0f, //Color yellow
            //1
            0.0f,0.0f,0.0f,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            0.0f+hW,0.0f,0.0f-hW,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            0.0f+hW-(hW/4.0f),0.0f,0.0f-hW,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            //2
            0.0f,0.0f,0.0f,
            0.0f, 0.0f, 1.0f, 1.0f,  //Color blue
            0.0f+hW,0.0f,0.0f+hW,
            0.0f, 0.0f, 1.0f, 1.0f,  //Color blue
            0.0f+hW-(hW/4.0f),0.0f,0.0f+hW,
            0.0f, 0.0f, 1.0f, 1.0f,  //Color blue
            //3
            0.0f,0.0f,0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, //Color yellow
            0.0f-hW,0.0f,0.0f+hW,
            1.0f, 1.0f, 0.0f, 1.0f, //Color yellow
            0.0f-hW+(hW/4.0f),0.0f,0.0f+hW,
            1.0f, 1.0f, 0.0f, 1.0f, //Color yellow
            //4
            0.0f,0.0f,0.0f,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            0.0f-hW,0.0f,0.0f-hW,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
            0.0f-hW+(hW/4.0f),0.0f,0.0f-hW,
            1.0f, 0.0f, 0.0f, 1.0f, //Color red
    };
    glBindBuffer(GL_ARRAY_BUFFER, GLBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(modelData),
                 modelData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ModelRollPitch::update(float rollDegree, float pitchDegree) {
    //TODO hack before demo
    rollDegree=0;
    pitchDegree=5;
#ifdef DEBUG_POSITION
    rollDegree=0;
    pitchDegree=20;
#endif
#ifdef ARTIFICIAL_MOVEMENT
    float random1 = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    if(random1>0.98){
        artRollReverse=!artRollReverse;
    }
    random1=static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    if(random1>0.98){
        artPitchReverse=!artPitchReverse;
    }
    if(artRoll>=20){
        artRollReverse=true;
    }
    if(artRoll<=-20){
        artRollReverse=false;
    }
    if(artPitch>=20){
        artPitchReverse=true;
    }
    if(artPitch<=-20){
        artPitchReverse=false;
    }
    float random2 = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    random2*=0.2f;
    float random3 = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
    random3*=0.2f;
    if(artRollReverse){
        artRoll-=random2;
    }else{
        artRoll+=random2;
    }
    if(artRollReverse){
        artPitch-=random3;
    }else{
        artPitch+=random3;
    }
    rollDegree=artRoll*-0.5f;
    pitchDegree=artPitch;
#endif
    if(!roll){
        rollDegree=0.0f;
    }
    if(invRoll){
        rollDegree*=-1.0f;
    }
    if(!pitch){
        pitchDegree=0.0f;
    }
    if(invPitch){
        pitchDegree*=-1.0f;
    }
    glm::mat4x4 translM = glm::translate(glm::mat4(1.0f),glm::vec3(mX,mY,mZ));
    glm::mat4x4 rotateM=glm::mat4(1.0f);
    rotateM=glm::rotate(rotateM,glm::radians(pitchDegree), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateM=glm::rotate(rotateM,glm::radians(rollDegree), glm::vec3(0.0f, 0.0f, 1.0f));
    mModelM=translM*rotateM;
}
//create 3PP is really specific FPV_VR.
void ModelRollPitch::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM,bool create3pp) {
    mGLRenderColor->beforeDraw(GLBuffer[0]);
    if(create3pp){
        mGLRenderColor->draw(mModelM*ViewM,ProjM,0,3+4*3);
    }else{
        mGLRenderColor->draw(ViewM*mModelM,ProjM,0,3+4*3);
    }
    mGLRenderColor->afterDraw();
}


