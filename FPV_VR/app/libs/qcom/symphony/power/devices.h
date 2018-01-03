// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifndef SYMPHONY_LIBPOWER_DEVICES_H
#define SYMPHONY_LIBPOWER_DEVICES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

  SYMPHONY_DEVICE_NONE            = 0x0,
  SYMPHONY_DEVICE_TYPE_CPU_BIG    = 0x01,
  SYMPHONY_DEVICE_TYPE_CPU_LITTLE = 0x02,
  SYMPHONY_DEVICE_TYPE_GPU        = 0x04,
  SYMPHONY_DEVICE_TYPE_HEXAGON    = 0x08
} symphony_device_type_t;

typedef unsigned int symphony_device_set_t;

#ifdef __cplusplus
}
#endif

#endif
