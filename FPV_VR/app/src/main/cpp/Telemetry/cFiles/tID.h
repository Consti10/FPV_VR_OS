//
// Created by Constantin on 21.10.2017.
//

#ifndef OSDTESTER_TELEMETRYIDS_H
#define OSDTESTER_TELEMETRYIDS_H

//ID List
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
//ID list

static const int ID_HOME_FIX_BOOL=27;
static const int ID_UNKNOWN=28;
static const int ID_PLACEHOLDER=29;

//End ID List
#endif //OSDTESTER_TELEMETRYIDS_H
