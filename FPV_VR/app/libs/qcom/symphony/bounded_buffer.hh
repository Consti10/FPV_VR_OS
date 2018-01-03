// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/queues/bounded_buffer.hh>
#include <symphony/internal/util/macros.hh>

namespace symphony {
namespace beta{

template<typename T>
class bounded_buffer{
public:
  typedef typename internal::bbuf::bounded_buf<T> container_type;
  typedef T value_type;

  SYMPHONY_DELETE_METHOD(bounded_buffer(bounded_buffer const&));
  SYMPHONY_DELETE_METHOD(bounded_buffer(bounded_buffer &&));
  SYMPHONY_DELETE_METHOD(bounded_buffer& operator=(bounded_buffer const&));
  SYMPHONY_DELETE_METHOD(bounded_buffer& operator=(bounded_buffer &&));

  explicit bounded_buffer(size_t log_size) : _c(log_size) {}

  bool push(value_type const& v) { return (_c.push(v) != 0 ? true:false); }

  bool pop(value_type& r) { return (_c.pop(r) != 0 ? true:false); }

private:
  container_type _c;
};

};
};
