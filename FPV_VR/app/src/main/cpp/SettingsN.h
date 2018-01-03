//
// Created by Constantin on 24.10.2017.
//

#ifndef OSDTESTER_SETTINGSWRAPPERN_H
#define OSDTESTER_SETTINGSWRAPPERN_H

#include <string>

//Stereo and VR Rendering
extern float S_InterpupilaryDistance;
extern float S_SceneScale;
extern bool S_DistortionCorrection;
extern float S_UndistortionData[7];

extern int S_GHT_MODE; //GHT==ground head tracking
extern bool S_GHT_X;
extern bool S_GHT_Y;
extern bool S_GHT_Z;
extern int S_AHT_MODE; //AHT==air head tracking
extern bool S_AHT_YAW;
extern bool S_AHT_ROLL;
extern bool S_AHT_PITCH;

//OSD Settings
extern bool S_OSD_TextElements;
extern bool S_OSD_CompasLadder;
extern bool S_OSD_CompasLadder_HomeArrow;
extern bool S_OSD_CompasLadder_InvertHeading;
extern bool S_OSD_HeightLadder;
extern int S_OSD_HeightLadder_WhichTelemetryValue;
extern bool S_OSD_HorizonModel;
extern bool S_OSD_OverlaysVideo;
extern bool S_OSD_ParseLTM;
extern bool S_OSD_ParseFRSKY;
extern bool S_OSD_ParseMAVLINK;
extern bool S_OSD_ParseRSSI;
extern bool S_OSD_Roll;
extern bool S_OSD_Pitch;
extern bool S_OSD_InvertRoll;
extern bool S_OSD_InvertPitch;
extern int S_LTMPort;
extern int S_FRSKYPort;
extern int S_MAVLINKPort;
extern int S_RSSIPort;

//Fonts
extern bool S_TE[20];
constexpr int TE_DFPS=0;
constexpr int TE_GLFPS=1;
constexpr int TE_TIME=2;
constexpr int TE_RX1=3;
constexpr int TE_RX2=4;
constexpr int TE_RX3=5;
constexpr int TE_BATT_P=6;
constexpr int TE_BATT_V=7;
constexpr int TE_BATT_A=8;
constexpr int TE_BATT_AH=9;
constexpr int TE_HOME_D=10;
constexpr int TE_VS=11;
constexpr int TE_HS=12;
constexpr int TE_LAT=13;
constexpr int TE_LON=14;
constexpr int TE_HEIGHT_B=15;
constexpr int TE_HEIGHT_GPS=16;
constexpr int TE_N_SATS=17;


#endif //OSDTESTER_SETTINGSWRAPPERN_H
