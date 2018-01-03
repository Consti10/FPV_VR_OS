// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifndef SYMPHONY_LIBPOWER_AFFINITY_H
#define SYMPHONY_LIBPOWER_AFFINITY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  symphony_affinity_cores_all = 0,
  symphony_affinity_cores_big,
  symphony_affinity_cores_little
} symphony_affinity_cores_t;

typedef enum {
  symphony_affinity_mode_allow_local_setting = 0,

  symphony_affinity_mode_override_local_setting,

} symphony_affinity_mode_t;

typedef enum {
  symphony_affinity_pin_threads_false = 0,
  symphony_affinity_pin_threads_true
} symphony_affinity_pin_threads_t;

typedef struct {
  symphony_affinity_cores_t cores;
  symphony_affinity_pin_threads_t pin_threads;
  symphony_affinity_mode_t mode;
} symphony_affinity_settings_t;

void symphony_affinity_set(const symphony_affinity_settings_t as);

void symphony_affinity_reset();

symphony_affinity_settings_t symphony_affinity_get();

typedef void (*symphony_func_ptr_t)(void* data);

void symphony_affinity_execute(const symphony_affinity_settings_t desired_aff, symphony_func_ptr_t f, void* args);

#ifdef __cplusplus
}
#endif

#endif
