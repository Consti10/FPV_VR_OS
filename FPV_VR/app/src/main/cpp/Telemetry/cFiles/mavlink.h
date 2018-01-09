
#ifndef FPV_VR_MAVLINK_H
#define FPV_VR_MAVLINK_H

#include "mavlink/mavlink.h"
#include "telemetry.h"

void mavlink_read(telemetry_data_t *td, uint8_t *buf, int buflen);

#endif
