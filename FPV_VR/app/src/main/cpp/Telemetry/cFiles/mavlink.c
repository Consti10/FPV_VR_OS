#include "mavlink.h"
#include "tID.h"
#include <stdio.h>
#include <unistd.h>

mavlink_status_t status;
mavlink_message_t msg;


void mavlink_read(float td[], uint8_t *buf, int buflen) {
	int i;
	for(i=0; i<buflen; i++) {
        uint8_t c = buf[i];
        if (mavlink_parse_char(0, c, &msg, &status)) {
            switch (msg.msgid){
                case MAVLINK_MSG_ID_GPS_RAW_INT:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_GPS_RAW_INT: ");
                    td[ID_N_FIX] = mavlink_msg_gps_raw_int_get_fix_type(&msg);
                    td[ID_N_SAT] = mavlink_msg_gps_raw_int_get_satellites_visible(&msg);
//					  td->hdop = mavlink_msg_gps_raw_int_get_eph(&msg);
//					  td->heading = mavlink_msg_gps_raw_int_get_cog(&msg)/100.0f;
//                                        td->altitude = mavlink_msg_gps_raw_int_get_alt(&msg)/1000.0f;
//                                        td->latitude = mavlink_msg_gps_raw_int_get_lat(&msg)/10000000.0f;
//                                        td->longitude = mavlink_msg_gps_raw_int_get_lon(&msg)/10000000.0f;
//					fprintf(stdout, "heading:%f  ", td->heading);
//					fprintf(stdout, "altitude:%f  ", td->altitude);
//					fprintf(stdout, "latitude:%f  ", td->latitude);
//					fprintf(stdout, "longitude:%f  ", td->longitude);
                    //fprintf(stdout, "GPS fix:%d  ", td->fix);
                    //fprintf(stdout, "GPS sats:%d  ", td->sats);
//					fprintf(stdout, "GPS hdop:%d  ", td->hdop);
                    break;
                case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_GLOBAL_POSITION_INT: ");
                    td[ID_YAW] = mavlink_msg_global_position_int_get_hdg(&msg)/100.0f;
                    td[ID_HEIGHT_BandGPS] = mavlink_msg_global_position_int_get_relative_alt(&msg)/1000.0f;
                    td[ID_LAT] = mavlink_msg_global_position_int_get_lat(&msg)/10000000.0f;
                    td[ID_LON] = mavlink_msg_global_position_int_get_lon(&msg)/10000000.0f;
                    //fprintf(stdout, "heading:%f  ", td->heading);
                    //fprintf(stdout, "altitude:%f  ", td->altitude);
                    //fprintf(stdout, "latitude:%f  ", td->latitude);
                    //fprintf(stdout, "longitude:%f  ", td->longitude);
                    break;
                case MAVLINK_MSG_ID_ATTITUDE:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_ATTITUDE: ");
                    td[ID_ROLL] = mavlink_msg_attitude_get_roll(&msg)*57.2958;
                    td[ID_PITCH] = mavlink_msg_attitude_get_pitch(&msg)*57.2958;
                    //fprintf(stdout, "roll:%d  ", td->roll);
                    //fprintf(stdout, "pitch:%d  ", td->pitch);
                    break;
                case MAVLINK_MSG_ID_SYS_STATUS:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_SYS_STATUS: ");
                    td[ID_BATT_V] = mavlink_msg_sys_status_get_voltage_battery(&msg)/1000.0f;
                    td[ID_BATT_A] = mavlink_msg_sys_status_get_current_battery(&msg)/100.0f;
                    //fprintf(stdout, "voltage:%f  ", td->voltage);
                    //fprintf(stdout, "ampere:%f  ", td->ampere);
                    break;
                case MAVLINK_MSG_ID_VFR_HUD:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_VFR_HUD: ");
                    td[ID_VS] = mavlink_msg_vfr_hud_get_groundspeed(&msg)*3.6f;
                    td[ID_PLACEHOLDER] = mavlink_msg_vfr_hud_get_airspeed(&msg)*3.6f;
//					td->heading = mavlink_msg_vfr_hud_get_heading(&msg)/100.0f;
                    //fprintf(stdout, "speed:%d  ", td->speed);
                    //fprintf(stdout, "airspeed:%d  ", td->airspeed);
//					fprintf(stdout, "heading:%f  ", td->heading);
                    break;
                case MAVLINK_MSG_ID_HEARTBEAT:
                    td[ID_NValidR]++;
                    //fprintf(stdout, "MAVLINK_MSG_ID_HEARTBEAT ");
                    break;
                default:
                    break;
            }
        }
    }
}

