
#include "OSDRenderer.h"
#include "../Helper/GLHelper.h"
#include "../SettingsN.h"

#define TAG "OSDRenderer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

OSDRenderer::OSDRenderer(TelemetryReceiver* telemetryReceiver,const GLRenderColor* glRenderColor,const GLRenderText* glRenderText) {
    mTelemtryReceiver=telemetryReceiver;
    mGLRenderColor=glRenderColor;
    mGLRenderText=glRenderText;
    if(S_OSD_HorizonModel){
        mModelRollPitch=new ModelRollPitch(mGLRenderColor,S_OSD_Roll,S_OSD_Pitch,S_OSD_InvertRoll,S_OSD_InvertPitch);
    }
    if(S_OSD_CompasLadder){
        mCompasLadder=new CompasLadder(mGLRenderColor,mGLRenderText,S_OSD_CompasLadder_HomeArrow,S_OSD_CompasLadder_InvertHeading);
    }
    if(S_OSD_HeightLadder){
        mHeightLadder=new HeightLadder(mGLRenderColor,mGLRenderText);
    }
    if(S_OSD_TextElements){
        mTextElements=new TextElements(mGLRenderText,mGLRenderColor,mTelemtryReceiver);
    }
    heightLadderTelemetryHeightSourceVal=S_OSD_HeightLadder_WhichTelemetryValue;
}

void OSDRenderer::placeGLElementsMono(float videoX,float videoY,float videoZ,float videoW,float videoH) {
    float textHeight=videoW*0.03f;
    if(mTextElements!=NULL){
        mTextElements->setWorldPosition(videoX, videoY, videoZ, videoW, videoH,textHeight);
    }
    if(mCompasLadder!=NULL){
        float width1=videoW/4.0f;
        float height1=width1*mCompasLadder->RATIO;
        float x1=videoX+(videoW-width1)*0.5f;
        float y1=videoY+videoH-textHeight*3.0f-height1;
        mCompasLadder->setWorldPosition(x1,y1,videoZ,width1,height1,0.01f);
    }
    if(mHeightLadder!=NULL){
        float height2=videoH/2.0f;
        float width2=height2/mHeightLadder->RATIO;
        float x2=videoX;
        float y2=videoY+(videoH-height2)/2.0f;
        mHeightLadder->setWorldPosition(x2,y2,videoZ,width2,height2,0.01f);
    }
    if(mModelRollPitch!=NULL){
        mModelRollPitch->setWorldPosition(0,0,videoZ,2);
    }
    checkGlError("OSDRenderer::placeGLElementsMono");
}
void OSDRenderer::placeGLElementsStereo(float videoX,float videoY,float videoZ,float videoW,float videoH) {
    float textHeight=videoW*0.05f;
    if(mTextElements!=NULL){
        if(S_OSD_OverlaysVideo) {
            mTextElements->setWorldPosition(videoX, videoY, videoZ, videoW, videoH,textHeight);
        }else{
            mTextElements->setWorldPosition(videoX, videoY-textHeight*3.0f, videoZ, videoW, videoH+textHeight*6,textHeight);
        }
    }
    if(mCompasLadder!=NULL){
        float width=videoW/3.0f;
        float height=width*mCompasLadder->RATIO;
        float x=videoX+videoW/3.0f;
        float y=videoY+videoH-(videoW/3.0f*mCompasLadder->RATIO);
        mCompasLadder->setWorldPosition(x,y,videoZ,width,height,0.015f);
    }
    if(mHeightLadder!=NULL){
        float height=videoH/2.0f;
        float width=height/mHeightLadder->RATIO;
        float x=videoX;
        float y=videoY+(videoH-height)/2.0f;
        mHeightLadder->setWorldPosition(x,y,videoZ,width,height,0.015f);
    }
    if(mModelRollPitch!=NULL){
        mModelRollPitch->setWorldPosition(0,0,videoZ,2);
    }
    checkGlError("OSDRenderer::placeGLElementsStereo");
}

void OSDRenderer::updateAndDrawElements(glm::mat4x4 ViewM,glm::mat4x4 ViewM2, glm::mat4x4 ProjM,bool create3PPerspective){
    if(mTextElements!=NULL){
        mTextElements->updateElementsGL();
        mTextElements->drawElementsGL(ViewM,ProjM);
    }
    if(mModelRollPitch!=NULL){
        mModelRollPitch->update(mTelemtryReceiver->getVal(TelemetryReceiver::ID_ROLL),TelemetryReceiver::ID_PITCH);
        glm::mat4x4 rot=glm::translate(glm::mat4x4(1),glm::vec3(0,0,0));
        if(create3PPerspective){
            mModelRollPitch->drawGL(ViewM2,ProjM,true);
        }else{
            mModelRollPitch->drawGL(ViewM,ProjM,false);
        }
    }
    if(mCompasLadder!=NULL){
        mCompasLadder->updateGL(mTelemtryReceiver->getVal(TelemetryReceiver::ID_YAW),0);
        mCompasLadder->drawGL(ViewM,ProjM);
    }
    if(mHeightLadder!=NULL){
        float h;
        switch(heightLadderTelemetryHeightSourceVal){
            case 0:h=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_B);
                break;
            case 1:h=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_GPS);
                break;
            case 2:h=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_BandGPS);
            default:
                h=1234;
                break;
        }
        mHeightLadder->updateGL(h);
        mHeightLadder->drawGL(ViewM,ProjM);
    }
}

void OSDRenderer::updateElementsGL() {
    if(mTextElements!=NULL){
        mTextElements->updateElementsGL();
    }
    if(mModelRollPitch!=NULL){
        mModelRollPitch->update(mTelemtryReceiver->getVal(TelemetryReceiver::ID_ROLL),TelemetryReceiver::ID_PITCH);
    }
    if(mCompasLadder!=NULL){
        mCompasLadder->updateGL(mTelemtryReceiver->getVal(TelemetryReceiver::ID_YAW),0);
    }
    if(mHeightLadder!=NULL){
        float val;
        switch(S_OSD_HeightLadder_WhichTelemetryValue){
            case 0:val=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_B);break; //Barometer
            case 1:val=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_GPS);break; //GPS
            case 2:val=mTelemtryReceiver->getVal(TelemetryReceiver::ID_HEIGHT_BandGPS);break; //Barometer and GPS (fusion)
            default:
                val=0;break;
        }
        mHeightLadder->updateGL(val);
    }
    checkGlError("OSDRenderer::updateGLElements");
}

void OSDRenderer::drawElementsGL(glm::mat4x4 ViewM,glm::mat4x4 ViewM2, glm::mat4x4 ProjM,bool create3PPerspective) {
    if(mTextElements!=NULL){
        mTextElements->drawElementsGL(ViewM,ProjM);
    }
    if(mModelRollPitch!=NULL){
        if(create3PPerspective){
            mModelRollPitch->drawGL(ViewM2,ProjM,true);
        }else{
            mModelRollPitch->drawGL(ViewM,ProjM,false);
        }
    }
    if(mCompasLadder!=NULL){
        mCompasLadder->drawGL(ViewM,ProjM);
    }
    if(mHeightLadder!=NULL){
        mHeightLadder->drawGL(ViewM,ProjM);
    }
    checkGlError("OSDRenderer::drawGLElements");
}


void OSDRenderer::start() {
    if(mTextElements!=NULL){
        mTextElements->startUpdating();
    }
}

void OSDRenderer::stop() {
    if(mTextElements!=NULL){
        mTextElements->stopUpdating();
    }
}

