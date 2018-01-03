// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <bitset>
#include <string>

#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {

enum class executor_device : size_t {
  unspecified,
  first,
  cpu = first,
  gpucl,
  gpugl,
  gputexture,
  hexagon,
  last = hexagon
};

class executor_device_bitset {
  std::bitset<static_cast<size_t>(executor_device::last)+1> _bs;

public:
  executor_device_bitset() :
    _bs()
  {}

  executor_device_bitset(std::initializer_list<executor_device> const& il) :
    _bs()
  {
    for(auto ed : il) {
      SYMPHONY_INTERNAL_ASSERT(ed <= executor_device::last, "Invalid ed=%zu", static_cast<size_t>(ed));
      auto ied = static_cast<size_t>(ed);
      _bs[ied] = true;
    }
  }

  void add(executor_device const& ed) {
    SYMPHONY_INTERNAL_ASSERT(ed <= executor_device::last, "Invalid ed=%zu", static_cast<size_t>(ed));
    auto ied = static_cast<size_t>(ed);
    _bs[ied] = true;
  }

  template<typename Fn>
  void for_each(Fn&& fn) const {
    for(auto ied = static_cast<size_t>(executor_device::first); ied <= static_cast<size_t>(executor_device::last); ied++) {
      auto ed = static_cast<executor_device>(ied);
      if(has(ed))
        fn(ed);
    }
  }

  template<typename Fn>
  void for_each_all(Fn&& fn) const {
    for(auto ied = static_cast<size_t>(executor_device::unspecified); ied <= static_cast<size_t>(executor_device::last); ied++) {
      auto ed = static_cast<executor_device>(ied);
      if(has(ed))
        fn(ed);
    }
  }

  void add(executor_device_bitset const& edb) {
    edb.for_each([&](executor_device ed)
                 {
                   this->add(ed);
                 });
  }

  void remove(executor_device const& ed) {
    SYMPHONY_INTERNAL_ASSERT(ed <= executor_device::last, "Invalid ed=%zu", static_cast<size_t>(ed));
    auto ied = static_cast<size_t>(ed);
    _bs[ied] = false;
  }

  bool has(executor_device const& ed) const {
    SYMPHONY_INTERNAL_ASSERT(ed <= executor_device::last, "Invalid ed=%zu", static_cast<size_t>(ed));
    auto ied = static_cast<size_t>(ed);
    return _bs[ied];
  }

  size_t count() const {
    return _bs.count();
  }
};

inline
std::string to_string(executor_device ed) {
  return std::string(ed == executor_device::unspecified ? "USP" :
                     (ed == executor_device::cpu ? "CPU" :
                      (ed == executor_device::gpucl ? "GPUCL" :
                       (ed == executor_device::gpugl ? "GPUGL" :
                        (ed == executor_device::gputexture ? "GPUTEXTURE" : "HEX")))));
}

inline
std::string to_string(executor_device_bitset const& edb) {
  std::string s = "{";
  edb.for_each_all([&](executor_device ed)
                   {
                     s += to_string(ed) + " ";
                   });
  s += "}";
  return s;
}

};
};
