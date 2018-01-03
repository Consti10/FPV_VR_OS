// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

namespace symphony {
namespace internal {

class exception_state {
private:

  std::vector<std::exception_ptr> _exceptions;

public:

  using exception_state_ptr = std::unique_ptr<symphony::internal::exception_state>;

  exception_state() :
    _exceptions(0) {}

  exception_state(const exception_state& other)
    : _exceptions(other._exceptions) {
  }

  exception_state(exception_state&& other)
    : _exceptions(std::move(other._exceptions)) {
  }

  exception_state& operator=(const exception_state& other) {
    if (this == &other)
      return *this;
    _exceptions = other._exceptions;
    return *this;
  }

  exception_state& operator=(exception_state&& other) {
    if (this == &other)
      return *this;
    _exceptions = std::move(other._exceptions);
    return *this;
  }

  void add(std::exception_ptr& eptr);
  void propagate(exception_state_ptr& e);
  void rethrow();
};

};
};
