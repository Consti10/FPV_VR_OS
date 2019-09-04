//Created by SettingsParser on ...

#ifndef CONSTI_10_100_IDOSD
#define CONSTI_10_100_IDOSD

class IDOSD{
public:
    static constexpr const char* OSD_MONO_LINE_WIDTH="Line width mono, in percentage.";
    static constexpr const char* OSD_STEREO_LINE_WIDTH="Line width stereo, in percentage";
    static constexpr const char* OSD_LINE_FILL_COLOR="Line fill (inner) color";
    static constexpr const char* OSD_LINE_OUTLINE_COLOR="Line outline (outer) color";
    static constexpr const char* OSD_TEXT_FONT_TYPE="Text font type";
    static constexpr const char* OSD_TEXT_FILL_COLOR1="Text fill color 1";
    static constexpr const char* OSD_TEXT_FILL_COLOR2="Text fill color 2";
    static constexpr const char* OSD_TEXT_FILL_COLOR3="Text fill color 3";
    static constexpr const char* OSD_TEXT_OUTLINE_COLOR="Text outline color";
    static constexpr const char* OSD_MONO_TEXT_OUTLINE_STRENGTH="Mono text outline strength";
    static constexpr const char* OSD_STEREO_TEXT_OUTLINE_STRENGTH="Stereo text outline strength";
    static constexpr const char* OSD_TRANSPARENT_BACKGROUND_STRENGTH="Transparent background strength";
    static constexpr const char* OSD_MONO_GLOBAL_SCALE="Mono global scale, in %. Default 100.";
    static constexpr const char* OSD_STEREO_GLOBAL_SCALE="Stereo global scale, in %. Default 100.";
    static constexpr const char* OSD_STEREO_FOVX_SCALE="(Stereo only) FOV X SCALE, in %. Default 80";
    static constexpr const char* OSD_STEREO_FOVY_SCALE="(Stereo only) FOV Y SCALE, in %. Default 100";
    static constexpr const char* AH_mode="AH mode";
    static constexpr const char* AH_scale="AH scale";
    static constexpr const char* AH_roll="AH roll";
    static constexpr const char* AH_pitch="AH pitch";
    static constexpr const char* AH_invRoll="AH invRoll";
    static constexpr const char* AH_invPitch="AH invPitch";
    static constexpr const char* CL_enable="CL enable";
    static constexpr const char* CL_scale="CL scale";
    static constexpr const char* CL_homeArrow="CL homeArrow";
    static constexpr const char* CL_invHeading="CL invHeading";
    static constexpr const char* CL_COGoverMAG="CL COGoverMAG";
    static constexpr const char* AL_enable="AL enable";
    static constexpr const char* AL_scale="AL scale";
    static constexpr const char* AL_sourceValue="AL sourceValue";
    static constexpr const char* SL_enable="SL enable";
    static constexpr const char* SL_scale="SL scale";
    static constexpr const char* SL_useKMHinsteadOfMS="SL use km/h instead of m/s";
    static constexpr const char* TW_BATT_P="TW BATT_P";
    static constexpr const char* TW_BATT_V="TW BATT V";
    static constexpr const char* TW_BATT_MAH_USED="TW_BATT_MAH_USED";
    static constexpr const char* TE1_SCALE="TE1 scale";
    static constexpr const char* TE1_GLFPS="TE1 OPENGL FPS (renderer)";
    static constexpr const char* TE1_DEC_FPS="TE1 DECODER FPS";
    static constexpr const char* TE1_DEC_LATENCY_DETAILED="TE1 DECODER LATENCY DETAILED (parsing,waitForInputB,decoding)";
    static constexpr const char* TE1_DEC_LATENCY_SUM="TE1 DECODER LATENCY_SUM";
    static constexpr const char* TE1_DEC_DBITRATE="TE1 DECODER BITRATE";
    static constexpr const char* TE1_TIME="TE1 TIME";
    static constexpr const char* TE1_RX_EZWB="TE1 RX EZWB (worst)";
    static constexpr const char* TE1_RX1="TE1 RX1 (not ez-wb)";
    static constexpr const char* TE1_BATT_P="TE1 BATT PERCENTAGE";
    static constexpr const char* TE1_BATT_V="TE1 BATT VOLTAGE";
    static constexpr const char* TE1_BATT_A="TE1 BATT AMPERE (current)";
    static constexpr const char* TE1_BATT_mAH="TE1 BATT mAH (used capacity)";
    static constexpr const char* TE1_HOME_DISTANCE="TE1 HOME DISTANCE";
    static constexpr const char* TE1_VS="TE1 VS (Vertical speed)";
    static constexpr const char* TE1_HS="TE1 HS (Horizontal speed)";
    static constexpr const char* TE1_LAT="TE1 LAT (Latitude)";
    static constexpr const char* TE1_LON="TE1 LON (Longitude)";
    static constexpr const char* TE1_ALTITUDE_GPS="TE1 ALTITUDE GPS";
    static constexpr const char* TE1_ALTITUDE_BARO="TE1 ALTITUDE BARO";
    static constexpr const char* TE1_N_SATS="TE1 N SATS (Visible for uav gps)";
    static constexpr const char* TE1_XXX="TE1 XXX (for future use)";
    static constexpr const char* TE2_SCALE="TE2 scale";
    static constexpr const char* TE2_EZWB_UPLINK_RSSI="TE2 EZWB UPLINK RSSI (rc over wbc)";
    static constexpr const char* TE2_EZWB_UPLINK_BLOCKS="TE2 EZWB UPLINK BLOCKS (rc over wbc)";
    static constexpr const char* TE2_STATUS_AIR="TE2 STATUS AIR";
    static constexpr const char* TE2_STATUS_GROUND="TE2 STATUS GROUND";
    static constexpr const char* TE2_HS="TE2 HS (Horizontal speed)";
    static constexpr const char* TE2_VS="TE2 VS (Vertical speed)";
    static constexpr const char* TE2_LAT="TE2 LAT (Latitude)";
    static constexpr const char* TE2_LON="TE2 LON (Longitude)";
    static constexpr const char* TE2_HOME_DISTANCE="TE2 HOME DISTANCE";
    static constexpr const char* TE2_SAT="TE2 SAT";
    static constexpr const char* TE2_BATT_P="TE2 BATT PERCENTAGE";
    static constexpr const char* TE2_BATT_V="TE2 BATT VOLTAGE";
    static constexpr const char* TE2_BATT_A="TE2 BATT AMPERE (Current)";
    static constexpr const char* TE2_EZWB_RSSI_WORST="TE2 EZWB RSSI WORST";
    static constexpr const char* TE2_EZWB_STRENGTH_P="TE2 EZWB STRENGTH P";
    static constexpr const char* TE2_EZWB_BLOCKS="TE2 EZWB BLOCKS";
    static constexpr const char* TE2_EZWB_RSSI_ADAPTERS_1to6="TE2 EZWB RSSI ADAPTERS 1to6";
    static constexpr const char* TE2_FLIGHT_MODE="TE2 FLIGHT MODE";
};

#endif //CONSTI_10_100_IDOSD
