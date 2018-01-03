/* #################################################################################################################
 * LightTelemetry protocol (LTM)
 *
 * Ghettostation one way telemetry protocol for really low bitrates (1200/2400 bauds).
 *
 * Protocol details: 3 different frames, little endian.
 *   G Frame (GPS position) (2hz @ 1200 bauds , 5hz >= 2400 bauds): 18BYTES
 *    0x24 0x54 0x47 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF  0xFF   0xC0
 *     $     T    G  --------LAT-------- -------LON---------  SPD --------ALT-------- SAT/FIX  CRC
 *   A Frame (Attitude) (5hz @ 1200bauds , 10hz >= 2400bauds): 10BYTES
 *     0x24 0x54 0x41 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xC0
 *      $     T   A   --PITCH-- --ROLL--- -HEADING-  CRC
 *   S Frame (Sensors) (2hz @ 1200bauds, 5hz >= 2400bauds): 11BYTES
 *     0x24 0x54 0x53 0xFF 0xFF  0xFF 0xFF    0xFF    0xFF      0xFF       0xC0
 *      $     T   S   VBAT(mv)  Current(ma)   RSSI  AIRSPEED  ARM/FS/FMOD   CRC
 * ################################################################################################################# */
#include <android/log.h>
#include "ltm.h"
#include "tID.h"


static uint8_t LTMserialBuffer[LIGHTTELEMETRY_GFRAMELENGTH-4];
static uint8_t LTMreceiverIndex;
static uint8_t LTMcmd;
static uint8_t LTMrcvChecksum;
static uint8_t LTMreadIndex;
static uint8_t LTMframelength;



uint8_t ltmread_u8()  {
  return LTMserialBuffer[LTMreadIndex++];
}

uint16_t ltmread_u16() {
  uint16_t t = ltmread_u8();
  t |= (uint16_t)ltmread_u8()<<8;
  return t;
}

uint32_t ltmread_u32() {
  uint32_t t = ltmread_u16();
  t |= (uint32_t)ltmread_u16()<<16;
  return t;
}

static enum _serial_state {
    IDLE,
    HEADER_START1,
    HEADER_START2,
    HEADER_MSGTYPE,
    HEADER_DATA
}
c_state = IDLE;

void ltm_read(float td[], uint8_t *buf, int buflen) {

    for(int i=0; i<buflen; ++i) {
        uint8_t c = buf[i];
        if (c_state == IDLE) {
            c_state = (c=='$') ? HEADER_START1 : IDLE;
        }
        else if (c_state == HEADER_START1) {
            c_state = (c=='T') ? HEADER_START2 : IDLE;
        }
        else if (c_state == HEADER_START2) {
            switch (c) {
                case 'G':
                    LTMframelength = LIGHTTELEMETRY_GFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                case 'A':
                    LTMframelength = LIGHTTELEMETRY_AFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                case 'S':
                    LTMframelength = LIGHTTELEMETRY_SFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                case 'O':
                    LTMframelength = LIGHTTELEMETRY_OFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                case 'N':
                    LTMframelength = LIGHTTELEMETRY_NFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                case 'X':
                    LTMframelength = LIGHTTELEMETRY_XFRAMELENGTH;
                    c_state = HEADER_MSGTYPE;
                    break;
                default:
                    c_state = IDLE;
            }
            LTMcmd = c;
            LTMreceiverIndex=0;
        }
        else if (c_state == HEADER_MSGTYPE) {
            if(LTMreceiverIndex == 0) {
                LTMrcvChecksum = c;
            }
            else {
                LTMrcvChecksum ^= c;
            }
            if(LTMreceiverIndex == LTMframelength-4) {   // received checksum byte
                if(LTMrcvChecksum == 0) {
                    ltm_check(td);
                    c_state = IDLE;
                }
                else { // wrong checksum, drop packet
                    c_state = IDLE;

                }
            }
            else LTMserialBuffer[LTMreceiverIndex++]=c;
        }
    }
}

// --------------------------------------------------------------------------------------
// Decoded received commands 
void ltm_check(float *td) {
    //__android_log_print(ANDROID_LOG_ERROR, "FPV_VR", "LTM CHECK");
    LTMreadIndex = 0;
    if (LTMcmd==LIGHTTELEMETRY_GFRAME)  {
        td[ID_LAT] = (float)((int32_t)ltmread_u32())/10000000;
        td[ID_LON] = (float)((int32_t)ltmread_u32())/10000000;
        uint8_t uav_groundspeedms = ltmread_u8();
        td[ID_VS] = (float)(uav_groundspeedms * 3.6f); // convert to kmh
        td[ID_HEIGHT_BandGPS] = (float)((int32_t)ltmread_u32())/100.0f;
        uint8_t ltm_satsfix = ltmread_u8();
        td[ID_N_SAT] = (ltm_satsfix >> 2) & 0xFF;
        td[ID_N_FIX] = ltm_satsfix & 0b00000011;

        td[ID_NValidR]++;
        /*printf("LTM G FRAME: ");
        printf("fix:%d  ", td->fix);
        printf("sats:%d  ", td->sats);
        printf("altitude:%f  ", td->altitude);
        printf("latitude:%f  ", td->latitude);
        printf("longitude:%f  ", td->longitude);
        printf("groundspeed:%d  ", td->speed);*/

    }else if (LTMcmd==LIGHTTELEMETRY_AFRAME)  {
        td[ID_PITCH] = (int16_t)ltmread_u16();
        td[ID_ROLL] =  (int16_t)ltmread_u16();
        td[ID_YAW] = (float)((int16_t)ltmread_u16());
        if (td[ID_YAW] < 0 ) td[ID_YAW] = td[ID_YAW] + 360; //convert from -180/180 to 0/360Â°
        td[ID_NValidR]++;
        /*printf("LTM A FRAME: ");
        printf("heading:%f  ", td->heading);
        printf("roll:%d  ", td->roll);
        printf("pitch:%d  ", td->pitch);*/

    }else if (LTMcmd==LIGHTTELEMETRY_OFRAME)  {
        td[ID_HOME_LATITUDE] = (double)((int32_t)ltmread_u32())/10000000;
        td[ID_HOME_LONGITUDE] = (double)((int32_t)ltmread_u32())/10000000;
        td[ID_HOME_ALTITUDE] = (float)((int32_t)ltmread_u32())/100.0f;
        td[ID_HOME_FIX_BOOL] = ltmread_u8();
        td[ID_NValidR]++;
        /*printf("LTM O FRAME: ");
        printf("home_altitude:%f  ", td->altitude);
        printf("home_latitude:%f  ", td->latitude);
        printf("home_longitude:%f  ", td->longitude);
        printf("osdon:%d  ", td->osdon);
        printf("home_fix:%d  ", td->home_fix);*/

//  }else if (LTMcmd==LIGHTTELEMETRY_XFRAME)  {

//    td->hdop = (float)((uint16_t)ltmread_u16())/10000.0f;
//    printf("LTM X FRAME:\n");
//    printf("GPS hdop:%f  ", td->hdop);

    }else if (LTMcmd==LIGHTTELEMETRY_SFRAME)  {
        td[ID_BATT_V] = (float)ltmread_u16()/1000.0f;
        td[ID_BATT_A] = (float)ltmread_u16()/1000.0f;
        td[ID_RX1] = ltmread_u8();

        uint8_t uav_airspeedms = ltmread_u8();
        td[ID_VS] = (float)(uav_airspeedms * 3.6f); // convert to kmh

        /*uint8_t ltm_armfsmode = ltmread_u8();
        td->uav_arm = ltm_armfsmode & 0b00000001;
        td->uav_failsafe = (ltm_armfsmode >> 1) & 0b00000001;
        td->uav_flightmode = (ltm_armfsmode >> 2) & 0b00111111;*/
        td[ID_NValidR]++;
        /*
        printf("LTM S FRAME: ");
        printf("voltage:%f  ", td->voltage);
        printf("ampere:%f  ", td->ampere);
        printf("rssi:%f  ", td->rssi);
        printf("airspeed:%f  ", td->airspeed);

        printf("arm:%d  ", td->uav_arm);
        printf("failsafe:%d  ", td->uav_failsafe);
        printf("flightmode:%d  ", td->uav_flightmode);*/
    }
}
