//
// Created by Constantin on 21.10.2017.
//

#define DEBUG_TELEMETRY

#ifndef OSDTESTER_TELEMETRYIDS_H
#define OSDTESTER_TELEMETRYIDS_H

#include <stdio.h>
#include <android/log.h>

typedef struct telemetry_data_t{
    uint32_t validmsgsrx;
    uint32_t validmsgrx_ltm;
    uint32_t validmsgrx_mavlink;
    uint32_t validmsgrx_frsky;
    uint32_t datarx;
    float voltage;
    float ampere;
    float gps_altitude;
    float baro_altitude;
    float gps_baro_altitude; //MAVLINK only
    double longitude;
    double latitude;
    float speed;
    float airspeed;
    uint32_t sats;
    uint32_t fix;
    float roll;
    float pitch;
    float heading;
    float rssi1;
    float rssi2;
/*#if defined(LTM) || defined(MAVLINK)
    int16_t roll, pitch;
	uint8_t rssi;
//	float hdop;
#endif*/
/*#if defined LTM
    // ltm S frame
	uint8_t status;
	uint8_t uav_arm;
	uint8_t uav_failsafe;
	uint8_t uav_flightmode;
// ltm N frame
	uint8_t gpsmode;
	uint8_t navmode;
	uint8_t navaction;
	uint8_t wpnumber;
	uint8_t naverror;
// ltm x frame
	uint8_t hw_status;
// ltm o frame
	float home_altitude;
	double home_longitude;
	double home_latitude;
	uint8_t osdon;
	uint8_t home_fix;
#endif*/


    //wifibroadcast_rx_status_t *rx_status;
    //wifibroadcast_rx_status_t_osd *rx_status_osd;
    //wifibroadcast_rx_status_t_rc *rx_status_rc;
}telemetry_data_t;

//using memset instead
/*void telemetry_init(telemetry_data_t *td) {
    td->validmsgsrx=0;
    td->validmsgrx_ltm=0;
    td->validmsgrx_mavlink=0;
    td->validmsgrx_frsky=0;
    td->datarx=0;
    td->voltage=0;
    td->ampere=0;
    td->gps_altitude=0;
    td->baro_altitude=0;
    td->gps_baro_altitude=0; //MAVLINK only
    td->longitude=0;
    td->latitude=0;
    td->speed=0;
    td->airspeed=0;
    td->sats=0;
    td->fix=0;
    td->roll=0;
    td->pitch=10;
    td->heading=0;
    td->rssi1=0;
    td->rssi2=0;
}*/

#endif //OSDTESTER_TELEMETRYIDS_H
