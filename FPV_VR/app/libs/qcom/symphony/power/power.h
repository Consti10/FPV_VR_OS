// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifndef SYMPHONY_LIBPOWER_POWER_H
#define SYMPHONY_LIBPOWER_POWER_H

#include <stdbool.h>

#include <symphony/power/devices.h>
#include <symphony/power/types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool symphony_power_init();

void symphony_power_terminate();

bool symphony_power_is_supported();

bool symphony_power_request_efficient_mode(symphony_milliseconds_t duration,
                                           symphony_device_set_t device_set);

bool symphony_power_request_normal_mode(symphony_device_set_t device_set);

bool symphony_power_request_perf_burst_mode(symphony_milliseconds_t duration,
                                            symphony_device_set_t device_set);

bool symphony_power_request_saver_mode(symphony_milliseconds_t duration,
                                       symphony_device_set_t device_set);

bool symphony_power_request_window_mode(symphony_power_freq_percent_t min_freq_percent,
                                        symphony_power_freq_percent_t max_freq_percent,
                                        symphony_milliseconds_t duration,
                                        symphony_device_set_t device_set);

void symphony_power_set_goal(symphony_power_goal_t desired,
                             symphony_power_tolerance_t tolerance,
                             symphony_device_set_t devices);

void symphony_power_clear_goal();

void symphony_power_regulate(symphony_power_goal_t measured);

symphony_power_goal_performance_t symphony_power_get_goal_performance();

#ifdef __cplusplus
}
#endif

#endif
