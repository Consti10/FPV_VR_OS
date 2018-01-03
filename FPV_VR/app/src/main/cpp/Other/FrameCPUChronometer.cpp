//
// Created by Constantin on 25.11.2017.
//

#include <android/log.h>
#include <sstream>
#include "FrameCPUChronometer.h"
#include "../Helper/Time.h"

#define TAG "FrameCPUChronometer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

//Defining RELEASE disables all measurements
//#define RELEASE

FrameCPUChronometer::FrameCPUChronometer(std::vector<std::string> timestampNames) {
#ifndef RELEASE
    for(unsigned long i=0;i<timestampNames.size();i++){
        FrameCPUTimestamp* frameCPUTimestamp=new FrameCPUTimestamp(timestampNames.at(i));
        mFrameCPUTimestamps.push_back(frameCPUTimestamp);
    }
    lastAvgLogMS=getTimeMS();
#endif
}

void FrameCPUChronometer::start(bool whichEye) {
#ifndef RELEASE
    if(whichEye){
        leftEyeLastStart=getSystemTimeNS();
    }else{
        rightEyeLastStart=getSystemTimeNS();
    }
#endif
}

void FrameCPUChronometer::setTimestamp(bool whichEye,int whichTimestamp) {
#ifndef RELEASE
    FrameCPUTimestamp* currentTS=mFrameCPUTimestamps.at((unsigned long)whichTimestamp);
    const int64_t  time=getSystemTimeNS();
    if(whichEye){
        if(whichTimestamp>0){
            FrameCPUTimestamp* lastTS=mFrameCPUTimestamps.at((unsigned long)(whichTimestamp-1));
            currentTS->leftEyeLastDelta=time-lastTS->leftEyeLastStop;
            currentTS->leftEyeLastStop=time;
        }else{
            currentTS->leftEyeLastDelta=time-leftEyeLastStart;
            currentTS->leftEyeLastStop=time;
        }
        currentTS->leftEyeDeltaSumUs+=(currentTS->leftEyeLastDelta/1000);
        currentTS->leftEyeDeltaC++;
    }else{
        if(whichTimestamp>0){
            FrameCPUTimestamp* lastTS=mFrameCPUTimestamps.at((unsigned long)(whichTimestamp-1));
            currentTS->rightEyeLastDelta=time-lastTS->rightEyeLastStop;
            currentTS->rightEyeLastStop=time;
        }else{
            currentTS->rightEyeLastDelta=time-rightEyeLastStart;
            currentTS->rightEyeLastStop=time;
        }
        currentTS->rightEyeDeltaSumUs+=(currentTS->rightEyeLastDelta/1000);
        currentTS->rightEyeDeltaC++;
    }
#endif
}

void FrameCPUChronometer::stop(bool whichEye) {
#ifndef RELEASE
    if(whichEye){
        leftEyeLastDelta=getSystemTimeNS()-leftEyeLastStart;
        leftEyeDeltaSumUs+=(leftEyeLastDelta/1000);
        leftEyeDeltaC++;
    }else{
        rightEyeLastDelta=getSystemTimeNS()-rightEyeLastStart;
        rightEyeDeltaSumUs+=(rightEyeLastDelta/1000);
        rightEyeDeltaC++;
    }
#endif
}

void FrameCPUChronometer::print() {
#ifndef RELEASE
    /*std::ostringstream oss;
    oss<<"......................CPU frame time......................";
    for(unsigned long i=0;i<mFrameCPUTimestamps.size();i++){
        FrameCPUTimestamp* currentTS=mFrameCPUTimestamps.at(i);
        double leftEyeDelta=(currentTS->leftEyeLastDelta/1000)/1000.0;
        double rightEyeDelta=(currentTS->rightEyeLastDelta/1000)/1000.0;
        double frameDelta=(leftEyeDelta+rightEyeDelta)/2.0;
        oss<<"\n";
        oss<<currentTS->name.c_str()<<": leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<frameDelta;
        //oss<<"xTx. leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | left&right:"<<frameDelta;
    }
    double leftEyeDelta=(leftEyeLastDelta/1000)/1000.0;
    double rightEyeDelta=(rightEyeLastDelta/1000)/1000.0;
    oss<<"\n"<<"CPUTimeSum:"<<" leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<(leftEyeDelta+rightEyeDelta)/2.0f;
    oss<<"\n----  -----  ----  ----  ----  ----  ----  ----  ";
    LOGV("%s",oss.str().c_str());*/
    if(getTimeMS()-lastAvgLogMS>5*1000){//every 5  seconds
        std::ostringstream oss2;
        oss2<<"....................Avg. CPU frame times....................";
        for(unsigned long i=0;i<mFrameCPUTimestamps.size();i++){
            FrameCPUTimestamp* currentTS=mFrameCPUTimestamps.at(i);
            int64_t leftEyeAvgDelta=currentTS->leftEyeDeltaSumUs/currentTS->leftEyeDeltaC;
            int64_t rightEyeAvgDelta=currentTS->rightEyeDeltaSumUs/currentTS->rightEyeDeltaC;
            double leftEyeDelta=leftEyeAvgDelta/1000.0;
            double rightEyeDelta=rightEyeAvgDelta/1000.0;
            double frameDelta=(leftEyeDelta+rightEyeDelta)/2.0;
            oss2<<"\n";
            oss2<<currentTS->name.c_str()<<": leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<frameDelta;
        }
        double leftEyeDelta=(leftEyeDeltaSumUs/leftEyeDeltaC)/1000.0;
        double rightEyeDelta=(rightEyeDeltaSumUs/rightEyeDeltaC)/1000.0;
        oss2<<"\n"<<"CPUTimeSum:"<<" leftEye:"<<leftEyeDelta<<" | rightEye:"<<rightEyeDelta<<" | avg:"<<(leftEyeDelta+rightEyeDelta)/2.0f;
        oss2<<"\n----  -----  ----  ----  ----  ----  ----  ----  ";
        LOGV("%s",oss2.str().c_str());
        lastAvgLogMS=getTimeMS();
    }
#endif
}

void FrameCPUChronometer::resetTS() {
#ifndef RELEASE
    for(unsigned long i=0;i<mFrameCPUTimestamps.size();i++){
        FrameCPUTimestamp* currentTS=mFrameCPUTimestamps.at(i);
        currentTS->reset();
    }
    leftEyeLastStart=0;rightEyeLastStart=0;
    leftEyeLastDelta=0;rightEyeLastDelta=0;
    leftEyeDeltaSumUs=0;leftEyeDeltaC=0;
    rightEyeDeltaSumUs=0;rightEyeDeltaC=0;
#endif
}


