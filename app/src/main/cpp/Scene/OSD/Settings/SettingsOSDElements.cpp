//
// Created by Constantin on 1/17/2019.
//

#include "SettingsOSDElements.h"
#include "IDOSD.hpp"
#include <jni.h>
#include <SharedPreferences.hpp>

SettingsOSDElements::SettingsOSDElements(JNIEnv *env, jobject androidContext) {
    SharedPreferences prefOSDElements(env,androidContext,"pref_osd");
    //Compass Ladder
    oCompassL.enable=prefOSDElements.getBoolean(IDOSD::CL_enable);
    oCompassL.scale=prefOSDElements.getInt(IDOSD::CL_scale);
    oCompassL.invHeading=prefOSDElements.getBoolean(IDOSD::CL_invHeading);
    oCompassL.homeArrow = prefOSDElements.getBoolean(IDOSD::CL_homeArrow);
    oCompassL.COGoverMag = prefOSDElements.getBoolean(IDOSD::CL_COGoverMAG);
    //Altitude ladder
    oAltitudeL.scale=prefOSDElements.getInt(IDOSD::AL_scale);
    oAltitudeL.enable=prefOSDElements.getBoolean(IDOSD::AL_enable);
    oAltitudeL.sourceVal = static_cast<VLAltitude::SOURCE_VAL >(prefOSDElements.getInt(IDOSD::AL_sourceValue));
    //Speed ladder
    oSpeedL.scale=prefOSDElements.getInt(IDOSD::SL_scale);
    oSpeedL.enable=prefOSDElements.getBoolean(IDOSD::SL_enable);
    //Artificial horizon
    oArtificialHorizon.mode=prefOSDElements.getInt(IDOSD::AH_mode);
    oArtificialHorizon.scale=prefOSDElements.getInt(IDOSD::AH_scale);
    oArtificialHorizon.roll=prefOSDElements.getBoolean(IDOSD::AH_roll);
    oArtificialHorizon.pitch=prefOSDElements.getBoolean(IDOSD::AH_pitch);
    oArtificialHorizon.invRoll=prefOSDElements.getBoolean(IDOSD::AH_invRoll);
    oArtificialHorizon.invPitch=prefOSDElements.getBoolean(IDOSD::AH_invPitch);
    //Text element warning
    oTextWarning.batteryPercentage=prefOSDElements.getBoolean(IDOSD::TW_BATT_P);
    oTextWarning.batteryVoltage=prefOSDElements.getBoolean(IDOSD::TW_BATT_V);
    oTextWarning.batteryMAHUsed=prefOSDElements.getBoolean(IDOSD::TW_BATT_MAH_USED);
//text elements 1 TE1 -----------------------------------------------------------------
    oTextElement1.scale=prefOSDElements.getInt(IDOSD::TE1_SCALE);
    oTextElement1.enableXX.clear();
    if(prefOSDElements.getBoolean(IDOSD::TE1_GLFPS)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::OPENGL_FPS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_DEC_FPS)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::DECODER_FPS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_DEC_DBITRATE)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::DECODER_BITRATE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_DEC_LATENCY_DETAILED)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::DECODER_LATENCY_DETAILED);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_DEC_LATENCY_SUM)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::DECODER_LATENCY_SUM);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_TIME)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::FLIGHT_TIME);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_RX_EZWB)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_RX1)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::RX_1);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_BATT_P)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::BATT_PERCENTAGE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_BATT_V)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::BATT_VOLTAGE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_BATT_A)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::BATT_CURRENT);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_BATT_mAH)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::BATT_USED_CAPACITY);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_HOME_DISTANCE)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::HOME_DISTANCE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_VS)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::VS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_HS)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::HS_GROUND);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_LAT)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::LATITUDE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_LON)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::LONGITUDE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_ALTITUDE_GPS)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::ALTITUDE_GPS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_ALTITUDE_BARO)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::ALTITUDE_BARO);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_N_SATS)){
        oTextElement1.enableXX.push_back(TelemetryReceiver::SATS_IN_USE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE1_XXX)){
        oTextElement1.enableXX.push_back( TelemetryReceiver::XXX);
    }
// text elements 2 TE2 -----------------------------------------------------------------
    oTextElement2.scale=prefOSDElements.getInt(IDOSD::TE2_SCALE);
    oTextElement2.enableXX.clear();
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_UPLINK_RSSI)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_UPLINK_RC_RSSI);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_UPLINK_BLOCKS)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_UPLINK_RC_BLOCKS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_STATUS_AIR)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_STATUS_AIR);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_STATUS_GROUND)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_STATUS_GROUND);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_VS)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::VS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_HS)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::HS_GROUND);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_LAT)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::LATITUDE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_LON)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::LONGITUDE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_HOME_DISTANCE)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::HOME_DISTANCE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_FLIGHT_MODE)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::FLIGHT_STATUS_MAV_ONLY);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_SAT)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::SATS_IN_USE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_BATT_A)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::BATT_CURRENT);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_BATT_P)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::BATT_PERCENTAGE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_BATT_V)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::BATT_VOLTAGE);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_RSSI_WORST)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_STRENGTH_P)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_DOWNLINK_VIDEO_RSSI2);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_BLOCKS)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_BLOCKS);
    }
    if(prefOSDElements.getBoolean(IDOSD::TE2_EZWB_RSSI_ADAPTERS_1to6)){
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER0);
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER1);
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER2);
        oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER3);
        //oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER4);
        //oTextElement2.enableXX.push_back(TelemetryReceiver::EZWB_RSSI_ADAPTER5);
    }
    //LOGD("%d %d %d",oSpeedL.scale,oCompassL.scale,oArtificialHorizon.scale);
}

