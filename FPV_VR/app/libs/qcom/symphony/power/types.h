// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#ifndef SYMPHONY_LIBPOWER_TYPES_H
#define SYMPHONY_LIBPOWER_TYPES_H

typedef unsigned int symphony_power_freq_t;
typedef unsigned int symphony_power_freq_percent_t;

typedef unsigned long long symphony_milliseconds_t;

typedef float symphony_power_goal_t;
typedef float symphony_power_tolerance_t;

typedef struct {
  unsigned long long _regulation_steps;
  double _goal_fraction;
  double _average_error;
  double _mean_squared_error;
  double _normalized_mserror;
} symphony_power_goal_performance_t;

#endif
