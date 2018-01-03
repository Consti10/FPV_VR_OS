// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/internal/device/cldevice.hh>
#include <symphony/internal/legacy/types.hh>

namespace symphony {
namespace internal {
namespace legacy {

template<typename Attribute>
inline void get_devices(Attribute d_type, std::vector<device_ptr>* devices)
{
  SYMPHONY_INTERNAL_ASSERT(devices, "null vector passed to get_devices");
  internal::get_devices(d_type, devices);
}

template<typename Attribute>
auto get_device_attribute(device_ptr device, Attribute attr) ->
     typename internal::device_attr<Attribute>::type
{
  auto d_ptr = internal::c_ptr(device);
  SYMPHONY_API_ASSERT(d_ptr, "null device_ptr");
  return d_ptr->get(std::forward<Attribute>(attr));
}

};
};
};

#endif
