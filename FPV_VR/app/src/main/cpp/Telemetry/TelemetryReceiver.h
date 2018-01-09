//
// Created by Constantin on 10.10.2017.
//

#ifndef OSDTESTER_TELEMETRYRECEIVER_H
#define OSDTESTER_TELEMETRYRECEIVER_H

#include <UDPReceiver.h>

#include <cFiles/telemetry.h>
struct homeData{
    double altitude;
    double longitude;
    double latitude;
};
struct otherOSDData{
    float decoder_fps;
    float opengl_fps;
    float time_seconds;
    float batt_percentage;
    float batt_ah;
    float home_distance_m;
    float home_heading;
    float rssi_ezwb;
};

class TelemetryReceiver {
public:
    TelemetryReceiver(bool recLTM,int ltmPort,bool recFRSKY,int frskyPort,bool recMAVLINK,int mavlinkPort,
                      bool recOther,int otherPort);
    void startReceiving();
    void stopReceiving();
    string statisticsToString();
    int getNReceivedTelemetryBytes();

    telemetry_data_t* get_uav_td();
    otherOSDData* get_other_osd_data();
    void recalculateHomeDistance();
    void recalculateHomeHeading();
    //LTM has a O frame, which contains home lat,lon,alt.
    //but currently we are only using the location provided by android
    void setHome(double latitude,double longitude,double attitude);

    //these are called via lambda by the UDP receiver(s)
    void onLTMDataReceivedCallback(uint8_t[],int);
    void onFRSKYDataReceivedCallback(uint8_t[],int);
    void onMAVLINKDataReceivedCallback(uint8_t[],int);
    void onOTHERDataReceivedCallback(uint8_t[],int);
private:
    bool receiveLTM,receiveFRSKY,receiveMAVLINK,receiveWBCTelemetry;
    int LTMPort,FRSKYPort,MAVLINKPort,OTHERPort;
    std::shared_ptr<UDPReceiver> mLTMReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mFRSKYReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mMAVLINKReceiver= nullptr;
    std::shared_ptr<UDPReceiver> mOTHERReceiver= nullptr;

    long nLTMBytes;
    long nFRSKYBytes;
    long nMAVLINKBytes;
    long nWIFIBRADCASTBytes;

    homeData home_data;
    otherOSDData other_osd_data;
    telemetry_data_t uav_td;
};


#endif //OSDTESTER_TELEMETRYRECEIVER_H
