// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <initializer_list>
#include <string>

#include <symphony/power/devices.h>

#include <symphony/internal/util/macros.hh>

namespace symphony {

class device_set;

namespace internal {
  symphony_device_set_t get_raw_device_set_t(device_set const& d);
};

enum device_type {
  cpu_big = SYMPHONY_DEVICE_TYPE_CPU_BIG,
  cpu_little = SYMPHONY_DEVICE_TYPE_CPU_LITTLE,
  cpu = SYMPHONY_DEVICE_TYPE_CPU_BIG | SYMPHONY_DEVICE_TYPE_CPU_LITTLE,
  gpu = SYMPHONY_DEVICE_TYPE_GPU,
  hexagon = SYMPHONY_DEVICE_TYPE_HEXAGON,
};

std::string to_string(device_type d);

class device_set {

  symphony_device_set_t _device_set_mask;

public:

  device_set();

  device_set(std::initializer_list<device_type> device_list);

  bool on_cpu() const;

  bool on_cpu_big() const;

  bool on_cpu_little() const;

  bool on_gpu() const;

  bool on_hexagon() const;

  device_set& add(device_type d);

  device_set& add(device_set const& other);

  device_set& remove(device_type d);

  device_set& remove(device_set const& other);

  device_set& negate();

  std::string to_string() const;

  SYMPHONY_DEFAULT_METHOD(device_set(device_set const&));

  SYMPHONY_DEFAULT_METHOD(device_set& operator=(device_set const&));

  SYMPHONY_DEFAULT_METHOD(device_set(device_set&&));

  SYMPHONY_DEFAULT_METHOD(device_set& operator=(device_set&&));

  friend symphony_device_set_t internal::get_raw_device_set_t(device_set const& d);
};

};
