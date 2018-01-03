//
// Created by Constantin on 06.11.2017.
//

#include <jni.h>
#include <UDPReceiver.h>
#include <TelemetryReceiver.h>
#include <sstream>
#include "../SettingsN.h"
#include "../Helper/CPUPriorities.h"

#define TAG "TestReceiverN"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

UDPReceiver* mVideoReceiver=NULL;
TelemetryReceiver* mTelemetryReceiver=NULL;
int mVideoPort;

//we only have to hold these values since nLTMBytes usw are hold by TelemetryReceiver. There is no VideoReceiver class (only VideoPlayer) since
//Receiving/Playing video is a cpp/java mix
long nVideoBytes;
int nVideoFrames,nVideoKeyFrames;

//int MODE=1;

uint8_t nalu_data[1024*1024];
int nalu_data_position=4;
int nalu_search_state=0;
void parseForNALUTest(uint8_t *data, int data_length) {
    for (int i = 0; i < data_length; ++i) {
        nalu_data[nalu_data_position++] = data[i];
        if (nalu_data_position == 1024*1024 - 1) {
            nalu_data_position = 0;
        }
        switch (nalu_search_state) {
            case 0:
            case 1:
            case 2:
                if (data[i] == 0)
                    nalu_search_state++;
                else
                    nalu_search_state = 0;
                break;
            case 3:
                if (data[i] == 1) {
                    nalu_data[0] = 0;
                    nalu_data[1] = 0;
                    nalu_data[2] = 0;
                    nalu_data[3] = 1;
                    //NALU found
                    nVideoFrames++;
                    nalu_data_position = 4;
                }
                nalu_search_state = 0;
                break;
            default:
                break;
        }
    }
}

void onVideoDataReceivedCallback(uint8_t data[],int data_length){
    nVideoBytes+=data_length;
    parseForNALUTest(data,data_length);
}

extern "C"{
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_TestReceiver_createAllReceiverN(
        JNIEnv *env, jobject obj,jint videoPort) {
    nalu_data_position=4;
    nalu_search_state=0;
    nVideoBytes=0;
    nVideoFrames=0;
    nVideoKeyFrames=0;
    mVideoPort=(int)videoPort;
    mVideoReceiver=new UDPReceiver(mVideoPort,UDPReceiver::MODE_BLOCKING,onVideoDataReceivedCallback,"TestRecN Video",CPU_PRIORITY_UDPRECEIVER_VIDEO);
    mVideoReceiver->startReceiving();
    mTelemetryReceiver=new TelemetryReceiver(S_OSD_ParseLTM,S_LTMPort,S_OSD_ParseFRSKY,S_FRSKYPort,S_OSD_ParseMAVLINK,
                                             S_MAVLINKPort,S_OSD_ParseRSSI,S_RSSIPort);
    mTelemetryReceiver->startReceiving();
}

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_TestReceiver_stopAndDeleteAllReceiverN(
        JNIEnv *env, jobject obj) {
    if(mVideoReceiver!=NULL){
        mVideoReceiver->stopReceiving();
        delete(mVideoReceiver);
        mVideoReceiver=NULL;
    }
    if(mTelemetryReceiver!=NULL){
        mTelemetryReceiver->stopReceiving();
        delete(mTelemetryReceiver);
        mTelemetryReceiver=NULL;
    }
}

JNIEXPORT jstring JNICALL Java_constantin_fpv_1vr_TestReceiver_getStringN(
        JNIEnv *env, jobject obj) {
    ostringstream ostringstream1;
    /*if(MODE==0){
        ostringstream1<<"Video:"<<(nVideoBytes/1024)<<"kB";
        ostringstream1<<"\nTelemetry:"<<(mTelemetryReceiver->getNReceivedTelemetryBytes()/1024)<<"kB";
    }else{*/
        ostringstream1<<"Listening for video on port "<<mVideoPort;
        ostringstream1<<"\nReceived: "<<nVideoBytes<<"B"<<" | parsed frames: "<<nVideoFrames<<" | key frames: "<<nVideoKeyFrames;
        ostringstream1<<"\n"<<mTelemetryReceiver->statisticsToString();
    //}
    jstring ret=env->NewStringUTF(ostringstream1.str().c_str());
    return ret;
}
/*JNIEXPORT void JNICALL Java_constantin_fpv_1vr_TestReceiver_setModeN(
        JNIEnv *env, jobject obj,jint mode) {
    MODE=mode;
}*/

JNIEXPORT jboolean JNICALL Java_constantin_fpv_1vr_TestReceiver_anyDataReceived(
        JNIEnv *env, jobject obj) {
    return (jboolean) (nVideoBytes+mTelemetryReceiver->getNReceivedTelemetryBytes() >0);
}

}