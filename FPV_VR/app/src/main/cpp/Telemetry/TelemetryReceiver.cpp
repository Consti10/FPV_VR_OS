//
// Created by Constantin on 10.10.2017.
//

#include <sstream>
#include <cstdlib>
#include <cFiles/telemetry.h>
#include <cmath>
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
    nFRSKYBytes=0;
    nMAVLINKBytes=0;
    nWIFIBRADCASTBytes=0;

    memset (&uav_td, 0, sizeof(uav_td));
    uav_td.pitch=10;
    memset (&other_osd_data, 0, sizeof(other_osd_data));
    memset (&home_data, 0, sizeof(home_data));
}

void TelemetryReceiver::startReceiving() {
    int mode=UDPReceiver::MODE_BLOCKING;
    int telemetryBuffsize=64;
    if(receiveLTM){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onLTMDataReceivedCallback(data,data_length);
        };
        mLTMReceiver=std::make_shared<UDPReceiver>(LTMPort,mode,"TelRN LTM",CPU_PRIORITY_UDPRECEIVER_TELEMETRY,telemetryBuffsize,f);
        mLTMReceiver->startReceiving();
    }
    if(receiveFRSKY){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onFRSKYDataReceivedCallback(data,data_length);
        };
        mFRSKYReceiver=std::make_shared<UDPReceiver>(FRSKYPort,mode,"TelRN FRSKY",CPU_PRIORITY_UDPRECEIVER_TELEMETRY,telemetryBuffsize,f);
        mFRSKYReceiver->startReceiving();
    }
    if(receiveMAVLINK){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onMAVLINKDataReceivedCallback(data,data_length);
        };
        mMAVLINKReceiver=std::make_shared<UDPReceiver>(MAVLINKPort,mode,"TelRN MAVLINK",CPU_PRIORITY_UDPRECEIVER_TELEMETRY,telemetryBuffsize,f);
        mMAVLINKReceiver->startReceiving();
    }
    if(receiveWBCTelemetry){
        std::function<void(uint8_t data[],int data_length)> f = [=](uint8_t data[],int data_length) {
            this->onOTHERDataReceivedCallback(data,data_length);
        };
        mOTHERReceiver=std::make_shared<UDPReceiver>(OTHERPort,mode,"TelRN WB",CPU_PRIORITY_UDPRECEIVER_TELEMETRY,telemetryBuffsize,f);
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

string TelemetryReceiver::statisticsToString() {
    std::ostringstream ostream;
    if(receiveLTM){
        ostream<<"\nListening for LTM telemetry on port "<<LTMPort;
        ostream<<"\nReceived: "<<nLTMBytes<<"B"<<" | Packets:"<<uav_td.validmsgrx_ltm;
        ostream<<"\n";
    }else{
        ostream<<"\nLTM telemetry disabled\n";
    }
    if(receiveFRSKY){
        ostream<<"\nListening for FRSKY telemetry on port "<<FRSKYPort;
        ostream<<"\nReceived: "<<nFRSKYBytes<<"B"<<" | Packets:"<<uav_td.validmsgrx_frsky;
        ostream<<"\n";
    }else{
        ostream<<"\nFRSKY telemetry disabled\n";
    }
    if(receiveMAVLINK){
        ostream<<"\nListening for MAVLINK telemetry on port "<<MAVLINKPort;
        ostream<<"\nReceived: "<<nMAVLINKBytes<<"B"<<" | Packets:"<<uav_td.validmsgrx_mavlink;
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

void TelemetryReceiver::onLTMDataReceivedCallback(uint8_t data[],int data_length){
    ltm_read(&uav_td,data,data_length);
    nLTMBytes+=data_length;
}

void TelemetryReceiver::onFRSKYDataReceivedCallback(uint8_t data[],int data_length){
    frsky_read(&uav_td,data,data_length);
    nFRSKYBytes+=data_length;
}

void TelemetryReceiver::onMAVLINKDataReceivedCallback(uint8_t data[],int data_length){
    mavlink_read(&uav_td,data,data_length);
    nMAVLINKBytes+=data_length;
}
void TelemetryReceiver::onOTHERDataReceivedCallback(uint8_t data[],int data_length){
    nWIFIBRADCASTBytes+=data_length;
    if(data_length>0){
        uint8_t ByteUint8_t = data[0];
        float rssi = (float) ByteUint8_t;
       other_osd_data.rssi_ezwb=rssi;
    }
    //LOGV("WBRSSI:%f",telemetryData[TelemetryReceiver::ID_RX1]);
}

int TelemetryReceiver::getNReceivedTelemetryBytes() {
    return (int)(nLTMBytes+nFRSKYBytes+nMAVLINKBytes+nWIFIBRADCASTBytes);
}

void TelemetryReceiver::setHome(double latitude, double longitude,double attitude) {
    home_data.latitude=latitude;
    home_data.longitude=longitude;
    home_data.altitude=attitude;
}

telemetry_data_t *TelemetryReceiver::get_uav_td() {
    return &uav_td;
}

otherOSDData *TelemetryReceiver::get_other_osd_data() {
    return &other_osd_data;
}

//taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L296
//changed slightly to use double precision
double distance_between(double lat1, double long1, double lat2, double long2) {
    // returns distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude. Uses great-circle
    // distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    double delta = (long1-long2)*0.017453292519;
    double sdlong = sin(delta);
    double cdlong = cos(delta);
    lat1 = (lat1)*0.017453292519;
    lat2 = (lat2)*0.017453292519;
    double slat1 = sin(lat1);
    double clat1 = cos(lat1);
    double slat2 = sin(lat2);
    double clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = delta*delta;
    delta += (clat2 * sdlong)*(clat2 * sdlong);
    delta = sqrt(delta);
    double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    return delta * 6372795;
}
//taken from tinygps: https://github.com/mikalhart/TinyGPS/blob/master/TinyGPS.cpp#L321
//changed slightly to use double precision
double course_to (double lat1, double long1, double lat2, double long2)
{
    // returns course in degrees (North=0, West=270) from position 1 to position 2,
    // both specified as signed decimal-degrees latitude and longitude.
    // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
    // Courtesy of Maarten Lamers
    double dlon = (long2-long1)*0.017453292519;
    lat1 = (lat1)*0.017453292519;
    lat2 = (lat2)*0.017453292519;
    double a1 = sin(dlon) * cos(lat2);
    double a2 = sin(lat1) * cos(lat2) * cos(dlon);
    a2 = cos(lat1) * sin(lat2) - a2;
    a2 = atan2(a1, a2);
    if (a2 < 0.0)
    {
        a2 += M_PI*2;
    }
    return (180.0/M_PI)*(a2);
}
void TelemetryReceiver::recalculateHomeDistance() {
    other_osd_data.home_distance_m=(float)distance_between(home_data.latitude,home_data.longitude,uav_td.latitude,uav_td.longitude);
}

void TelemetryReceiver::recalculateHomeHeading() {
    //TODO: this is called in each frame from OpenGL. So we should check if the data has changed since the last update before actually performing the calculation
    other_osd_data.home_heading=(float)course_to(uav_td.latitude,uav_td.longitude,home_data.latitude,home_data.longitude);
}

