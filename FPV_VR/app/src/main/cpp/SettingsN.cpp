
#include "SettingsN.h"
#include <jni.h>
#include <android/log.h>

#define TAG "SettingsN"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

//Stereo and VR Rendering
float S_InterpupilaryDistance;
float S_SceneScale;
bool S_DistortionCorrection;
float S_UndistortionData[7];

//head tracking
int S_GHT_MODE; //GHT==ground head tracking
bool S_GHT_X;
bool S_GHT_Y;
bool S_GHT_Z;
int S_AHT_MODE; //AHT==air head tracking
bool S_AHT_YAW;
bool S_AHT_ROLL;
bool S_AHT_PITCH;

//OSD Settings
bool S_OSD_TextElements;
bool S_OSD_CompasLadder;
bool S_OSD_CompasLadder_HomeArrow;
bool S_OSD_CompasLadder_InvertHeading;
bool S_OSD_HeightLadder;
int S_OSD_HeightLadder_WhichTelemetryValue;
bool S_OSD_HorizonModel;
bool S_OSD_OverlaysVideo;
bool S_OSD_ParseLTM;
bool S_OSD_ParseFRSKY;
bool S_OSD_ParseMAVLINK;
bool S_OSD_ParseRSSI;
bool S_OSD_Roll;
bool S_OSD_Pitch;
bool S_OSD_InvertRoll;
bool S_OSD_InvertPitch;
int S_LTMPort;
int S_FRSKYPort;
int S_MAVLINKPort;
int S_RSSIPort;

//Fonts
bool S_TE[20];
//acces Individual booleans by S_OSD_TE_XXX



extern "C" {

/*JNIEXPORT void JNICALL Java_constantin_osdtester_Settings_initializeN(
        JNIEnv *env, jobject obj) {
    S_InterpupilaryDistance=0.4f;
    S_SceneScale=100;
    S_DistortionCorrection=true;
    for(int i=0;i<7;i++){
        S_UndistortionData[i]=0;
    }
    //OSD Settings
    S_OSD_TextElements = true;
    S_OSD_CompasLadder = true;
    S_OSD_HeightLadder = true;
    S_OSD_HorizonModel = true;
    S_OSD_CLHomeArrow = true;
    S_OSD_OverlaysVideo = true;
    S_OSD_ParseLTM = true;
    S_OSD_ParseFRSKY = true;
    S_OSD_ParseMAVLINK = true;
    S_OSD_ParseRSSI = true;
    S_OSD_Roll = true;
    S_OSD_Pitch = true;
    S_OSD_InvertRoll = true;
    S_OSD_InvertPitch = true;
    S_LTMPort=5001;
    S_FRSKYPort=5002;
    S_MAVLINKPort=5003;
    S_RSSIPort=5004;
    for(int i=0;i<20;i++){
        S_TE[i]=true;
    }
}*/

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_Settings_setBoolByStringID(
        JNIEnv *env, jobject obj,jboolean jval,jstring jstring1) {
    bool val=(bool)jval;
    const char *cparam = env->GetStringUTFChars(jstring1,0);
    std::string s(cparam);
    //LOGV("Set Bool by string: %s",s.c_str());
    if("DistortionCorrection"==s){
        S_DistortionCorrection=val;
    } else if("TextElements"==s){
        S_OSD_TextElements=val;
    }else if("CompassLadder"==s){
        S_OSD_CompasLadder=val;
    }else if("CLInvertHeading"==s){
        S_OSD_CompasLadder_InvertHeading=val;
    }else if("CLHomeArrow"==s){
        S_OSD_CompasLadder_HomeArrow = val;
    } else if("HeightLadder"==s){
        S_OSD_HeightLadder=val;
    }else if("HorizonModel"==s){
        S_OSD_HorizonModel=val;
    }else if("OverlaysVideo"==s){
        S_OSD_OverlaysVideo=val;
    }else if("ParseLTM"==s){
        S_OSD_ParseLTM=val;
    }else if("ParseFRSKY"==s){
        S_OSD_ParseFRSKY=val;
    }else if("ParseMAVLINK"==s){
        S_OSD_ParseMAVLINK=val;
    }else if("ParseRSSI"==s){
        S_OSD_ParseRSSI=val;
    }else if("Roll"==s){
        S_OSD_Roll=val;
    }else if("Pitch"==s){
        S_OSD_Pitch=val;
    }else if("InvertRoll"==s){
        S_OSD_InvertRoll=val;
    }else if("InvertPitch"==s){
        S_OSD_InvertPitch=val;
    }
//Head tracking booleans
    else if("GroundHeadTrackingX"==s){
        S_GHT_X=val;
    }
    else if("GroundHeadTrackingY"==s){
        S_GHT_Y=val;
    }
    else if("GroundHeadTrackingZ"==s){
        S_GHT_Z=val;
    }
    else if("AirHeadTrackingYaw"==s){
        S_AHT_YAW=val;
    }
    else if("AirHeadTrackingRoll"==s){
        S_AHT_ROLL=val;
    }
    else if("AirHeadTrackingPitch"==s){
        S_AHT_PITCH=val;
    }
//TextElements Values
    else if("TE_DFPS"==s){
        S_TE[TE_DFPS]=val;
    }else if("TE_GLFPS"==s){
        S_TE[TE_GLFPS]=val;
    }else if("TE_TIME"==s){
        S_TE[TE_TIME]=val;
    }else if("TE_RX1"==s){
        S_TE[TE_RX1]=val;
    }else if("TE_RX2"==s){
        S_TE[TE_RX2]=val;
    }else if("TE_RX3"==s){
        S_TE[TE_RX3]=val;
    }else if("TE_BATT_P"==s){
        S_TE[TE_BATT_P]=val;
    }else if("TE_BATT_V"==s){
        S_TE[TE_BATT_V]=val;
    }else if("TE_BATT_A"==s){
        S_TE[TE_BATT_A]=val;
    }else if("TE_BATT_AH"==s){
        S_TE[TE_BATT_AH]=val;
    }else if("TE_HOME_D"==s){
        S_TE[TE_HOME_D]=val;
    }else if("TE_VS"==s){
        S_TE[TE_VS]=val;
    }else if("TE_HS"==s){
        S_TE[TE_HS]=val;
    }else if("TE_LAT"==s){
        S_TE[TE_LAT]=val;
    }else if("TE_LON"==s){
        S_TE[TE_LON]=val;
    }else if("TE_HEIGHT_B"==s){
        S_TE[TE_HEIGHT_B]=val;
    }else if("TE_HEIGHT_GPS"==s){
        S_TE[TE_HEIGHT_GPS]=val;
    }else if("TE_N_SATS"==s){
        S_TE[TE_N_SATS]=val;
    }else{
        LOGV("Native Bool Value not specified: %s",s.c_str());
    }
    env->ReleaseStringUTFChars(jstring1, cparam);
}
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_Settings_setFloatByStringID(
        JNIEnv *env, jobject obj,jfloat jval,jstring jstring1) {
    float val=(float)jval;
    const char *cparam = env->GetStringUTFChars(jstring1,0);
    std::string s(cparam);
    if("InterpupilaryDistance"==s){
        S_InterpupilaryDistance=val;
    }else if("SceneScale"==s){
        S_SceneScale=val;
    }else{
        LOGV("Native Float Value not specified: %s",s.c_str());
    }
}
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_Settings_setIntByStringID(
        JNIEnv *env, jobject obj,jint jval,jstring jstring1) {
    int val = (int) jval;
    const char *cparam = env->GetStringUTFChars(jstring1, 0);
    std::string s(cparam);
    if ("LTMPort" == s) {
        S_LTMPort = val;
    }else if ("FRSKYPort" == s) {
        S_FRSKYPort = val;
    }else if ("MAVLINKPort" == s) {
        S_MAVLINKPort = val;
    }else if ("RSSIPort" == s) {
        S_RSSIPort = val;
    }else if ("HLSelectSourceValue" == s){
        S_OSD_HeightLadder_WhichTelemetryValue = val;
    }else if ("GroundHeadTrackingMode" == s) {
        S_GHT_MODE = val;
    }else if ("AirHeadTrackingMode" == s) {
        S_AHT_MODE = val;
    }



    else{
        LOGV("Native Int Value not specified: %s",s.c_str());
    }
}

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_Settings_setUndistortionData(
        JNIEnv *env, jobject obj,jfloat maxRSq,jfloat k1,jfloat k2,jfloat k3,jfloat k4,jfloat k5,jfloat k6) {
    S_UndistortionData[0]=maxRSq;
    S_UndistortionData[1]=k1;
    S_UndistortionData[2]=k2;
    S_UndistortionData[3]=k3;
    S_UndistortionData[4]=k4;
    S_UndistortionData[5]=k5;
    S_UndistortionData[6]=k6;
}




}
