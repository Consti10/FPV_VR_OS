//
// Created by Constantin on 10.10.2017.
//

#include <sstream>
#include <cstdlib>
#include "TelemetryReceiver.h"
#include "../Helper/CPUPriorities.h"

extern "C"{
#include <cFiles/ltm.h>
#include <cFiles/frsky.h>
#include <cFiles/mavlink.h>
}
#define TAG "TelemetryReceiver"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)


TelemetryReceiver::TelemetryReceiver(bool recLTM,int ltmPort,bool recFRSKY,int frskyPort,bool recMAVLINK,int mavlinkPort,
                                     bool recOther,int otherPort) {
    receiveLTM=recLTM;
    receiveFRSKY=recFRSKY;
    receiveMAVLINK=recMAVLINK;
    receiveWBCTelemetry=recOther;
    LTMPort=ltmPort;
    FRSKYPort=frskyPort;
    MAVLINKPort=mavlinkPort;
    OTHERPort=otherPort;

    nLTMBytes=0;
    nLTMPackets=0;
    nFRSKYBytes=0;
    nFRSKYPackets=0;
    nMAVLINKBytes=0;
    nMAVLINKPackets=0;
    nWIFIBRADCASTBytes=0;

    for(int i=0;i<40;i++){
        telemetryData[i]=0;
    }
}

void TelemetryReceiver::startReceiving() {
    int mode=UDPReceiver::MODE_BLOCKING;
    if(receiveLTM){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onLTMDataReceivedCallback(data,data_length);
        };
        mLTMReceiver=std::make_shared<UDPReceiver>(LTMPort,mode,f,"TelRN LTM",CPU_PRIORITY_UDPRECEIVER_TELEMETRY);
        mLTMReceiver->startReceiving();
    }
    if(receiveFRSKY){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onFRSKYDataReceivedCallback(data,data_length);
        };
        mFRSKYReceiver=std::make_shared<UDPReceiver>(FRSKYPort,mode,f,"TelRN FRSKY",CPU_PRIORITY_UDPRECEIVER_TELEMETRY);
        mFRSKYReceiver->startReceiving();
    }
    if(receiveMAVLINK){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onMAVLINKDataReceivedCallback(data,data_length);
        };
        mMAVLINKReceiver=std::make_shared<UDPReceiver>(MAVLINKPort,mode,f,"TelRN MAVLINK",CPU_PRIORITY_UDPRECEIVER_TELEMETRY);
        mMAVLINKReceiver->startReceiving();
    }
    if(receiveWBCTelemetry){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onOTHERDataReceivedCallback(data,data_length);
        };
        mOTHERReceiver=std::make_shared<UDPReceiver>(OTHERPort,mode,f,"TelRN WB",CPU_PRIORITY_UDPRECEIVER_TELEMETRY);
        mOTHERReceiver->startReceiving();
    }
}

void TelemetryReceiver::stopReceiving() {
    if(receiveLTM){
        mLTMReceiver->stopReceiving();
        mLTMReceiver=NULL;
    }
    if(receiveFRSKY){
        mFRSKYReceiver->stopReceiving();
        mFRSKYReceiver=NULL;
    }
    if(receiveMAVLINK){
        mMAVLINKReceiver->stopReceiving();
        mMAVLINKReceiver=NULL;
    }
    if(receiveWBCTelemetry){
        mOTHERReceiver->stopReceiving();
        mOTHERReceiver=NULL;
    }
}

float TelemetryReceiver::getVal(int ID) {
    return telemetryData[ID];
}

void TelemetryReceiver::setVal(float val, int ID) {
    telemetryData[ID]=val;
}

string TelemetryReceiver::statisticsToString() {
    std::ostringstream ostream;
    if(receiveLTM){
        ostream<<"\nListening for LTM telemetry on port "<<LTMPort;
        ostream<<"\nReceived: "<<nLTMBytes<<"B"<<" | Packets:"<<nLTMPackets;
        ostream<<"\n";
    }else{
        ostream<<"\nLTM telemetry disabled\n";
    }
    if(receiveFRSKY){
        ostream<<"\nListening for FRSKY telemetry on port "<<FRSKYPort;
        ostream<<"\nReceived: "<<nFRSKYBytes<<"B"<<" | Packets:"<<nFRSKYPackets;
        ostream<<"\n";
    }else{
        ostream<<"\nFRSKY telemetry disabled\n";
    }
    if(receiveMAVLINK){
        ostream<<"\nListening for MAVLINK telemetry on port "<<MAVLINKPort;
        ostream<<"\nReceived: "<<nMAVLINKBytes<<"B"<<" | Packets:"<<nMAVLINKPackets;
        ostream<<"\n";
    }else{
        ostream<<"\nMAVLINK telemetry disabled\n";
    }
    if(receiveWBCTelemetry){
        ostream<<"\nListening for WIFIBROADCAST telemetry on port "<<OTHERPort;
        ostream<<"\nReceived: "<<nWIFIBRADCASTBytes<<"B";
        ostream<<"\n";
    }else{
        ostream<<"\nWIFIBROADCAST telemetry disabled\n";
    }
    return ostream.str();
}

void TelemetryReceiver::feedWithTestLTMData(uint8_t *data, int data_length) {
    ltm_read(telemetryData,data,data_length);
    nLTMBytes+=data_length;
}

void TelemetryReceiver::onLTMDataReceivedCallback(uint8_t data[],int data_length){
    ltm_read(telemetryData,data,data_length);
    nLTMBytes+=data_length;
}

void TelemetryReceiver::onFRSKYDataReceivedCallback(uint8_t data[],int data_length){
    frsky_read(telemetryData,data,data_length);
    nFRSKYBytes+=data_length;
}

void TelemetryReceiver::onMAVLINKDataReceivedCallback(uint8_t data[],int data_length){
    mavlink_read(telemetryData,data,data_length);
    nMAVLINKBytes+=data_length;
}
void TelemetryReceiver::onOTHERDataReceivedCallback(uint8_t data[],int data_length){
    nWIFIBRADCASTBytes+=data_length;
    if(data_length>0){
        uint8_t ByteUint8_t = data[0];
        float rssi = (float) ByteUint8_t;
        telemetryData[TelemetryReceiver::ID_RX1]=rssi;
    }
    //LOGV("WBRSSI:%f",telemetryData[TelemetryReceiver::ID_RX1]);
}

int TelemetryReceiver::getNReceivedTelemetryBytes() {
    return (int)(nLTMBytes+nFRSKYBytes+nMAVLINKBytes+nWIFIBRADCASTBytes);
}

void TelemetryReceiver::setHome(double latitude, double longitude,double attitude) {
    telemetryData[ID_HOME_LATITUDE]=(float)latitude;
    telemetryData[ID_HOME_LONGITUDE]=(float)longitude;
    telemetryData[ID_HOME_ALTITUDE]=(float)attitude;
}
