//
// Created by Constantin on 10.10.2017.
//

#ifndef OSDTESTER_TELEMETRYRECEIVER_H
#define OSDTESTER_TELEMETRYRECEIVER_H

#include <UDPReceiver.h>

class TelemetryReceiver {
public:
    TelemetryReceiver(bool recLTM,int ltmPort,bool recFRSKY,int frskyPort,bool recMAVLINK,int mavlinkPort,
                      bool recOther,int otherPort);
    void startReceiving();
    void stopReceiving();
    float getVal(int ID);
    void setVal(float val,int ID);
    string statisticsToString();
    int getNReceivedTelemetryBytes();

    //LTM has a O frame, which contains home lat,lon,alt
    void setHome(double latitude,double longitude,double attitude);

    void feedWithTestLTMData(uint8_t data[],int data_length);
    //
    void onLTMDataReceivedCallback(uint8_t[],int);
    void onFRSKYDataReceivedCallback(uint8_t[],int);
    void onMAVLINKDataReceivedCallback(uint8_t[],int);
    void onOTHERDataReceivedCallback(uint8_t[],int);
    //
//ID List. All telemetry data is hold in a float[] array
    const static int ID_FPSD=0;
    const static int ID_FPSGL=1;
    const static int ID_TIME=2;
    const static int ID_RX1=3;
    const static int ID_RX2=4;
    const static int ID_RX3=5;
    const static int ID_BATT_PERC=6; //does not get written by any protocol
    const static int ID_BATT_V=7;
    const static int ID_BATT_A=8;
    const static int ID_BATT_AH=9;  //does not get written by any protocol
    const static int ID_HOME_M=10; //does not get written by any protocol
    const static int ID_VS=11;
    const static int ID_HS=12;
    const static int ID_LAT=13;
    const static int ID_LON=14;
    const static int ID_HEIGHT_B=15;
    const static int ID_HEIGHT_GPS=16;
    const static int ID_HEIGHT_BandGPS=17;
    const static int ID_YAW=18;
    const static int ID_ROLL=19;
    const static int ID_PITCH=20;
    const static int ID_N_SAT=21;
    const static int ID_NValidR=22;
    const static int ID_N_FIX=23;

    const static int ID_HOME_ALTITUDE=24;
    const static int ID_HOME_LONGITUDE=25;
    const static int ID_HOME_LATITUDE=26;
//End ID List
private:
    bool receiveLTM,receiveFRSKY,receiveMAVLINK,receiveWBCTelemetry;
    int LTMPort,FRSKYPort,MAVLINKPort,OTHERPort;
    std::shared_ptr<UDPReceiver> mLTMReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mFRSKYReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mMAVLINKReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mOTHERReceiver= nullptr;

    long nLTMBytes;
    int nLTMPackets;
    long nFRSKYBytes;
    int nFRSKYPackets;
    long nMAVLINKBytes;
    int nMAVLINKPackets;
    long nWIFIBRADCASTBytes;
    float telemetryData[40];

    double homeLatitude=0,homeLongitude=0;
};


#endif //OSDTESTER_TELEMETRYRECEIVER_H
