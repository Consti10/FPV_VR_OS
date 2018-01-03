// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/queues/bounded_lfqueue.hh>
#include <symphony/internal/util/macros.hh>

namespace symphony {

template<typename T>
class bounded_lfqueue{
public:
  typedef typename internal::blfq::blfq_size_t<T, (sizeof(size_t) >= sizeof(T))> container_type;
  typedef T value_type;

  SYMPHONY_DELETE_METHOD(bounded_lfqueue(bounded_lfqueue const&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue(bounded_lfqueue &&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue& operator=(bounded_lfqueue const&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue& operator=(bounded_lfqueue &&));

  explicit bounded_lfqueue(size_t log_size) : _c(log_size) {}

  bool push(value_type const& v) { return (_c.push(v, false) != 0 ? true:false); }

  bool pop(value_type& r) { return (_c.pop(r, false) != 0 ? true:false); }

private:
  container_type _c;
};

};

