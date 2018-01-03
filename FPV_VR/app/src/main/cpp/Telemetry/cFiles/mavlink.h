
#ifndef FPV_VR_MAVLINK_H
#define FPV_VR_MAVLINK_H

#include "mavlink/mavlink.h"

void mavlink_read(float td[], uint8_t *buf, int buflen);

#endif
