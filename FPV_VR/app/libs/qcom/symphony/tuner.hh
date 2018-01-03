// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/runtime.hh>

namespace symphony {
namespace pattern {

enum class shape {
  uniform,
  exponential,
  gaussian,
  triangle,
  inv_triangle,
  parabola,
  randif,
  hill,
  valley
};

class tuner {
public:
  using load_type = size_t;

  tuner() :
    _max_doc(symphony::internal::num_execution_contexts())
    , _min_chunk_size(1)
    , _static(false)
    , _shape(shape::uniform)
    , _serialize(false)
    , _user_setbit(false)
    , _cpu_load(0)
    , _hexagon_load(0)
    , _gpu_load(0)
    , _profile(false)
    {
      SYMPHONY_INTERNAL_ASSERT(_max_doc > 0, "Degree of Concurrency must be > 0!");
      SYMPHONY_INTERNAL_ASSERT(_min_chunk_size > 0, "Chunk size must be > 0!");
    }

  tuner& set_max_doc(size_t doc) {
    SYMPHONY_API_ASSERT(doc > 0, "Degree of concurrency must be > 0!");

    _max_doc = doc;
    return *this;
  }

  size_t get_doc() const {
    return _max_doc;
  }

  tuner& set_chunk_size(size_t sz) {
    SYMPHONY_API_ASSERT(sz > 0, "Chunk size must be > 0!");

    _min_chunk_size = sz;

    _user_setbit = true;
    return *this;
  }

  size_t get_chunk_size() const {
    return _min_chunk_size;
  }

  bool is_chunk_set() const {
    return _user_setbit;
  }

  tuner& set_static() {
    _static = true;
    return *this;
  }

  tuner& set_dynamic() {
    _static = false;
    return *this;
  }

  bool is_static() const {
    return _static;
  }

  tuner& set_shape(const symphony::pattern::shape& s) {
    _shape = s;
    return *this;
  }

  symphony::pattern::shape get_shape() const {
    return _shape;
  }

  tuner& set_serial() {
    _serialize = true;
    return *this;
  }

  bool is_serial() const {
    return _serialize;
  }

  tuner& set_cpu_load(load_type load) {
    _cpu_load = load;
    return *this;
  }

  load_type get_cpu_load() const {
    return _cpu_load;
  }

  tuner& set_hexagon_load(load_type load) {
    _hexagon_load = load;
    return *this;
  }

  load_type get_hexagon_load() const {
    return _hexagon_load;
  }

  tuner& set_gpu_load(load_type load) {
    _gpu_load = load;
    return *this;
  }

  load_type get_gpu_load() const {
    return _gpu_load;
  }

  tuner& set_profile() {
    _profile = true;
    return *this;
  }

  bool has_profile() const {
    return _profile;
  }

private:
  size_t _max_doc;
  size_t _min_chunk_size;
  bool   _static;
  shape  _shape;
  bool   _serialize;
  bool   _user_setbit;
  load_type _cpu_load;
  load_type _hexagon_load;
  load_type _gpu_load;
  bool   _profile;
};

};
};
