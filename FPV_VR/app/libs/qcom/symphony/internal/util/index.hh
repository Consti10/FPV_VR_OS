// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>

#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {

template<size_t Dims>
class index;

template<size_t Dims>
class index_base
{
protected:
  std::array<size_t, Dims> _data;
public:
  index_base() : _data() { _data.fill(0); }

  explicit index_base(const std::array<size_t, Dims>& rhs) : _data(rhs) { }

  size_t& operator [] (size_t i) { return _data[i]; }

  const size_t& operator [] (size_t i) const { return _data[i]; }

  virtual ~index_base() { }

  const std::array<size_t, Dims>& data() const { return _data; }

  bool operator == (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) == rhs.data());
  }

  bool operator != (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) == rhs.data());
  }

  bool operator < (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) < rhs.data());
  }

  bool operator <= (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) <= rhs.data());
  }

  bool operator > (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) > rhs.data());
  }

  bool operator >= (const index_base<Dims>& rhs) const {
    return (const_cast<std::array<size_t, Dims>&>(_data) >= rhs.data());
  }

  index<Dims>& operator += (const index<Dims>& rhs) {
    for(size_t i = 0; i < Dims; i++) {
      _data[i] += rhs[i];
    }
    return (*this);
  }

  index<Dims>& operator -= (const index<Dims>& rhs) {
    for(size_t i = 0; i < Dims; i++) {
      _data[i] -= rhs[i];
    }
    return (*this);
  }

  index<Dims> operator - (const index<Dims>& rhs) {
    return (index<Dims>(*this) -= rhs);
  }

  index<Dims> operator + (const index<Dims>& rhs) {
    return (index<Dims>(*this) += rhs);
  }

  index_base<Dims>& operator = (const index_base<Dims>& rhs) {
    _data = rhs.data();
    return (*this);
  }
};

template<size_t Dims>
class index;

template<>
class index<1> : public index_base<1>
{
public:
  index() : index_base<1>() { }

  explicit index(const std::array<size_t, 1>& rhs) : index_base<1>(rhs) { }

  explicit index(size_t i) {
     _data[0] = i;
  }

  void print() const {
    SYMPHONY_ALOG("(%zu)", _data[0]);
  }
};

template<>
class index<2> : public index_base<2>
{
public:
  index() : index_base<2>() { }

  explicit index(const std::array<size_t, 2>& rhs) : index_base<2>(rhs) { }

  index(size_t i, size_t j) {
     _data[0] = i;
     _data[1] = j;
  }

  void print() const {
    SYMPHONY_ALOG("(%zu, %zu)", _data[0], _data[1]);
  }
};

template<>
class index<3> : public index_base<3>
{
public:
  index() : index_base<3>() { }

  explicit index(const std::array<size_t, 3>& rhs) : index_base<3>(rhs) { }

  index(size_t i, size_t j, size_t k) {
     _data[0] = i;
     _data[1] = j;
     _data[2] = k;
  }

  void print() const {
    SYMPHONY_ALOG("(%zu, %zu, %zu)", _data[0], _data[1], _data[2]);
  }
};

};
};
