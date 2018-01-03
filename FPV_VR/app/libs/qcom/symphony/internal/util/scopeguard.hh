// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

namespace symphony {
namespace internal {

template <typename NullaryFn>
struct scope_guard {
   scope_guard(NullaryFn&& fn)
    : _active(true),
      _fn(std::move(fn)) {}

   scope_guard(scope_guard&& other)
    : _active(other._active),
      _fn(std::move(other._fn)) {
    other.reset();
  }

  SYMPHONY_DELETE_METHOD(scope_guard());
  SYMPHONY_DELETE_METHOD(scope_guard(const scope_guard&));
  SYMPHONY_DELETE_METHOD(scope_guard& operator=(scope_guard const&));

  void reset() {
    _active = false;
  }

  ~scope_guard() {
    if (_active)
      _fn();
  }

private:
  bool _active;
  NullaryFn const _fn;
};

template <typename NullaryFn>
inline scope_guard<NullaryFn> make_scope_guard(NullaryFn&& fn)
{
  return scope_guard<NullaryFn>(std::forward<NullaryFn>(fn));
}

};
};
