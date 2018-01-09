
#include "TextElements.h"
#include "../Helper/StringHelper.h"
#include "../Helper/CPUPriorities.h"
#include "../Helper/Time.h"
#include "../SettingsN.h"
#include <../Helper/GeometryHelper.h>
#include <unistd.h>
#include <cFiles/telemetry.h>

#define TAG "TextElements"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

TextElements::TextElements(const GLRenderText* glRenderText,const GLRenderColor* glRenderColor,TelemetryReceiver* telemetryReceiver) {
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderText=glRenderText;
    mGLRenderColor=glRenderColor;
    mTelemetryR=telemetryReceiver;
    for(int i=0;i<MAX_N_TEXT_E;i++){
        //enable[i]=true;
        enable[i]=S_TE[i];
    }
    glGenBuffers(1,mGLTextB);
    mCircularRefreshT=NULL;
    pthread_mutex_init(&runningLock,NULL);
    textElementsVertexDataAlreadyCreated=false;
}
TextElements::~TextElements() {
    stopUpdating();
    LOGV("Delete Text elements");
}

void TextElements::setWorldPosition(float x,float y,float z,float width,float height,float textH) {
    flightStartMS=getTimeMS();
    //If the circularRefreshThread is already running, we have to stop it while we are updating the vertex data
    //At the end of setWorldPosition, it will be restarted
    stopUpdating();
    //Two rectangles filled with text. One on top and one on the bottom of the video.
    float width1=width;
    float height1=textH*3.0f;
    float x1=x;
    float y1=y+height-3.0f*textH;
    float z1=z;
    float width2=width;
    float height2=textH*3.0f;
    float x2=x;
    float y2=y+3.0f*textH-textH;
    float z2=z;
#ifdef DEBUG_POSITION
    float debug[7*6*2];
    makeColoredRect(debug,0,glm::vec3(x1,y1,z1),glm::vec3(width1,0,0),glm::vec3(0,height1,0),0,1,0,1);
    makeColoredRect(debug,7*6,glm::vec3(x2,y2,z2),glm::vec3(width2,0,0),glm::vec3(0,height2,0),0,1,0,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug),
                 debug, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    mTextStack.clear();
    int nRowsInTopRect=3;
    int nRowsInBottomRect=3;
    float r=1,g=1,b=1;
    int idxTE=0,idxVect=0;
    for(int colum=0;colum<3;colum++){
        for(int row=0;row<nRowsInTopRect;row++){
            if(enable[idxTE]){
                //The user enabled the TextE
                GLTextObj* tmp=new GLTextObj("Initializing",x1+(width1/3.0f)*colum,y1+height1-textH-textH*row,z1,textH,r,g,b);
                mTextStack.push_back(tmp);
                linkVecToTE[idxVect]=idxTE;
                idxVect++;
            }
            idxTE++;
        }
    }
    for(int colum=0;colum<3;colum++){
        for(int row=0;row<nRowsInBottomRect;row++){
            if(enable[idxTE]){
                //The user enabled the TextE
                GLTextObj* tmp=new GLTextObj("Initializing",x2+(width2/3.0f)*colum,y2-textH*row,z2,textH,r,g,b);
                mTextStack.push_back(tmp);
                linkVecToTE[idxVect]=idxTE;
                idxVect++;
            }
            idxTE++;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, mGLTextB[0]);
    glBufferData(GL_ARRAY_BUFFER, 15*6*5*MAX_N_TEXT_E*sizeof(float),
                 NULL, GL_DYNAMIC_DRAW);
    for(int i=0;i<mTextStack.size();i++){
        GLTextObj* tmp=mTextStack.at((unsigned long)i);
        mGLRenderText->convertStringToVECs_UVs(tmp);
        glBufferSubData(GL_ARRAY_BUFFER,i*15*6*5*sizeof(float),15*6*5*sizeof(float),tmp->VECS_UVS);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    textElementsVertexDataAlreadyCreated=true;
    //After we have our vertex data, we can start our circular refresh thread
    startUpdating();
}

void TextElements::updateElementsGL() {
    //float flightTimeMin=((float)(getTimeMS()-flightStartMS))/1000.0f/60.0f;
    glBindBuffer(GL_ARRAY_BUFFER, mGLTextB[0]);
    unsigned long size=mTextStack.size();
    for(unsigned long i=0;i<size;i++){
        GLTextObj* tmp=mTextStack.at(i);
        if(tmp->newVecs_UVScalculated){
            if(pthread_mutex_trylock(&tmp->lock)==0){
                glBufferSubData(GL_ARRAY_BUFFER,i*15*6*5*sizeof(float),tmp->NVertices*5*sizeof(float),tmp->VECS_UVS);
                tmp->newVecs_UVScalculated=false;
                pthread_mutex_unlock(&tmp->lock);
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextElements::drawElementsGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM,ProjM,0,3*2*2);
    mGLRenderColor->afterDraw();
#endif
    mGLRenderText->beforeDraw(mGLTextB[0]);
    unsigned long size=mTextStack.size();
    for(unsigned long i=0;i<size;i++){
        GLTextObj* tmp=mTextStack.at(i);
        mGLRenderText->draw(ViewM,ProjM,6*15*(int)i,tmp->NVertices,
                            tmp->R,tmp->G,tmp->B);
    }
    mGLRenderText->afterDraw();
}

void TextElements::circularRefreshThread() {
    setCPUPriority(CPU_PRIORITY_TEXTELEMENTS_GLREFRESH,"TextElements");
    cMilliseconds lastUpdate=getTimeMS();
    while(running){
        for(unsigned long i=0;i<mTextStack.size();i++){
            if(!running){
                break;
            }
            GLTextObj* tmp=mTextStack.at(i);
            while(tmp->newVecs_UVScalculated){
                if(!running){
                    break;
                }
                usleep((useconds_t)20); //sleep 20us
            }
            pthread_mutex_lock(&tmp->lock);
            tmp->Text=getStringForTE(linkVecToTE[i]);
            mGLRenderText->convertStringToVECs_UVs(tmp);
            tmp->newVecs_UVScalculated=true;
            pthread_mutex_unlock(&tmp->lock);
            int64_t delta=getTimeMS()-lastUpdate;
            lastUpdate=getTimeMS();
            int64_t timeToSleep=10-delta; //update in 10ms intervals
            if(timeToSleep>0){
                if(!running){
                    break;
                }
                usleep((useconds_t)timeToSleep*1000);
            }
            //LOGV("Hi circular refresh");
        }
    }
}

string TextElements::getStringForTE(int TE) {
    string s="";
    float tmp;
    telemetry_data_t* uav_td=mTelemetryR->get_uav_td();
    otherOSDData* other_osd_data=mTelemetryR->get_other_osd_data();
    switch (TE){
        case TE_DFPS:s.append("Dec:");
            s.append(intToString((int)round(other_osd_data->decoder_fps),4));
            s.append("fps");
            break;
        case TE_GLFPS:s.append("OGL:");
            s.append(intToString((int)round(other_osd_data->opengl_fps),4));
            s.append("fps");
            break;
        case TE_TIME:s.append("Time:");
            tmp=(((float)(getTimeMS()-flightStartMS))/1000.0f);
            other_osd_data->time_seconds=tmp;
            if(tmp<60){
                s.append(floatToString(tmp,4,0));
                s.append("sec");
            }else {
                s.append(floatToString(tmp/60,4,0));
                s.append("min");
            }
            break;
        case TE_RX1:s.append("ezWB:");
            s.append(floatToString(other_osd_data->rssi_ezwb,7,0));
            s.append("rssi");
            break;
        case TE_RX2:s.append("RX1:");
            s.append(floatToString(uav_td->rssi1,7,0));
            s.append("rssi");
            break;
        case TE_RX3:s.append("RX2:");
            s.append(floatToString(uav_td->rssi2,7,0));
            s.append("rssi");
            break;
        case TE_BATT_P:s.append("Batt:");
            s.append(floatToString(other_osd_data->batt_percentage,9,0));
            s.append("%");
            break;
        case TE_BATT_V:s.append("Batt:");
            s.append(floatToString(uav_td->voltage,9,0));
            s.append("V");
            break;
        case TE_BATT_A:
            s.append("Batt:");
            s.append(floatToString(uav_td->ampere,9,0));
            s.append("A");
            break;
        case TE_BATT_AH:
            s.append("Batt:");
            s.append(floatToString(other_osd_data->batt_ah,9,0));
            s.append("Ah");
            break;
        case TE_HOME_D:s.append("Home:");
            mTelemetryR->recalculateHomeDistance();
            tmp=other_osd_data->home_distance_m;
            if(tmp>1000){
                tmp/=1000.0f;
                s.append(floatToString(tmp,6,0));
                s.append("km");
            }else{
                s.append(floatToString(tmp,7,0));
                s.append("m");
            }
            break;
        case TE_SPEED_V:s.append("VS:");
            s.append(floatToString(uav_td->speed,7,0));
            s.append("km/h");
            break;
        case TE_SPEED_H:s.append("VS:");
            s.append(floatToString(uav_td->airspeed,7,0));
            s.append("km/h");
            break;
        case TE_LAT:s.append("Lat:");
            s.append(floatToString((float)uav_td->latitude,9,0));
            //s.append("");
            break;
        case TE_LON:s.append("Lon:");
            s.append(floatToString((float)uav_td->longitude,9,0));
            //s.append("");
            break;
        case TE_HEIGHT_B:s.append("Height:");
            s.append(floatToString((float)uav_td->baro_altitude,7,0));
            s.append("m");
            break;
        case TE_HEIGHT_GPS:s.append("Height:");
            s.append(floatToString(uav_td->gps_altitude,7,0));
            s.append("m");
            break;
        case TE_N_SAT:s.append("Sats:");
            s.append(intToString((int)uav_td->sats,3));
            break;
        default:
            break;
    }
    return s;
}

//can be called externally and is called internally in setWorldPosition
void TextElements::startUpdating() {
    pthread_mutex_lock(&runningLock);
    if(mCircularRefreshT==NULL){
        if(!textElementsVertexDataAlreadyCreated){
            //it makes no sense to start the circular refresh thread before vertex data has been created.
            //But if startUpdating() is called before setWorldPosition that's the case.
            //just return,since setWorldPosition() also starts the circular refresh thread
            LOGV("No text objects to update are available");
        }else{
            running=true;
            mCircularRefreshT=new thread([this] { this->circularRefreshThread(); });
        }
    }else{
        LOGV("already running");
    }
    pthread_mutex_unlock(&runningLock);
    LOGV("startUpdating end");
}

void TextElements::stopUpdating() {
    pthread_mutex_lock(&runningLock);
    running=false;
    if(mCircularRefreshT!=NULL){
        if(mCircularRefreshT->joinable()){
            mCircularRefreshT->join();
        }
        delete(mCircularRefreshT);
    }else{
        LOGV("already stopped");
    }
    mCircularRefreshT=NULL;
    pthread_mutex_unlock(&runningLock);
    LOGV("stopUpdating end");
}

int TextElements::getNActiveTE() {
    int nActiveE=0;
    for(int i=0;i<MAX_N_TEXT_E;i++){
        if(enable[i]){
            nActiveE++;
        }
    }
    return nActiveE;
}


