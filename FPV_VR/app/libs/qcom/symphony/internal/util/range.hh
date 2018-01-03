// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <stdexcept>
#include <vector>

#include <symphony/exceptions.hh>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/index.hh>

namespace symphony {

namespace internal {

template<size_t Dims>
bool in_bounds(const std::array<size_t, Dims>& b,
               const std::array<size_t, Dims>& e,
               const std::array<size_t, Dims>& it)
{
  for(size_t i = 0; i < Dims; i++) {
    if(!((b[i] <= it[i]) && (it[i] < e[i]))) {
      return false;
    }
  }
  return true;
}

template<size_t Dims>
bool is_valid_index(const std::array<size_t, Dims>& b,
                    const std::array<size_t, Dims>& s,
                    const std::array<size_t, Dims>& it)
{
  for (size_t i = 0; i < Dims; i++) {
    if ((it[i] - b[i]) % s[i] != 0) {
      return false;
    }
  }
  return true;
}

template<size_t Dims>
struct check_bounds;

template<>
struct check_bounds<1>
{
  static void check(const std::array<size_t, 1>& b,
                    const std::array<size_t, 1>& e,
                    const std::array<size_t, 1>& it) {
    bool flag = in_bounds(b, e, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::out_of_range,
                          "it: [%zu], lb: [%zu], ub: [%zu]",
                          it[0], b[0], e[0]);
  }
};

template<>
struct check_bounds<2>
{
  static void check(const std::array<size_t, 2>& b,
                    const std::array<size_t, 2>& e,
                    const std::array<size_t, 2>& it) {
    bool flag = in_bounds(b, e, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::out_of_range,
                          "it: [%zu, %zu], lb: [%zu, %zu], ub: [%zu, %zu]",
                          it[0], it[1], b[0], b[1], e[0], e[1]);
  }
};

template<>
struct check_bounds<3>
{
  static void check(const std::array<size_t, 3>& b,
                    const std::array<size_t, 3>& e,
                    const std::array<size_t, 3>& it) {
    bool flag = in_bounds(b, e, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::out_of_range,
                          "it: [%zu, %zu, %zu], lb: [%zu, %zu, %zu],"
                          "ub: [%zu, %zu, %zu]", it[0], it[1], it[2],
                          b[0], b[1], b[2], e[0], e[1], e[2]);
  }
};

template<size_t Dims>
struct check_stride;

template<>
struct check_stride<1>
{
  static void check(const std::array<size_t, 1>& b,
                    const std::array<size_t, 1>& s,
                    const std::array<size_t, 1>& it) {
    bool flag = is_valid_index(b, s, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::invalid_argument,
                          "it: [%zu], lb: [%zu], stride: [%zu]",
                          it[0], b[0], s[0]);
  }
};

template<>
struct check_stride<2>
{
  static void check(const std::array<size_t, 2>& b,
                    const std::array<size_t, 2>& s,
                    const std::array<size_t, 2>& it) {
    bool flag = is_valid_index(b, s, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::invalid_argument,
                          "it: [%zu, %zu], lb: [%zu, %zu], stride: [%zu, %zu]",
                          it[0], it[1], b[0], b[1], s[0], s[1]);
  }
};

template<>
struct check_stride<3>
{
  static void check(const std::array<size_t, 3>& b,
                    const std::array<size_t, 3>& s,
                    const std::array<size_t, 3>& it) {
    bool flag = is_valid_index(b, s, it);
    SYMPHONY_API_THROW_CUSTOM(flag,
                          std::invalid_argument,
                          "it: [%zu, %zu, %zu], lb: [%zu, %zu, %zu],"
                          "stride: [%zu, %zu, %zu]", it[0], it[1], it[2],
                           b[0], b[1], b[2], s[0], s[1], s[2]);
  }
};

template<size_t Dims>
class range_base
{
protected:
  std::array<size_t, Dims>  _b;
  std::array<size_t, Dims>  _e;
  std::array<size_t, Dims>  _s;

public:
  range_base() : _b(), _e(), _s() {
    for (auto& ss: _s) ss = 1;
  }

  range_base(const std::array<size_t, Dims>& bb,
             const std::array<size_t, Dims>& ee) : _b(bb), _e(ee), _s() {
    for (auto& ss: _s) ss = 1;
  }
  range_base(const std::array<size_t, Dims>& bb,
             const std::array<size_t, Dims>& ee,
             const std::array<size_t, Dims>& ss) : _b(bb), _e(ee), _s(ss) { }

  size_t begin(const size_t i) const {
    SYMPHONY_API_ASSERT(i < Dims, "Index out of bounds");
    return _b[i];
  }

  size_t end(const size_t i) const {
    SYMPHONY_API_ASSERT(i < Dims, "Index out of bounds");
    return _e[i];
  }

  size_t stride(const size_t i) const {
    SYMPHONY_API_ASSERT(i < Dims, "Index out of bounds");
    return _s[i];
  }

  inline size_t num_elems(const size_t i) const {
    SYMPHONY_INTERNAL_ASSERT(_e[i] - _b[i], "begin == end");
    return ((_e[i] + (_s[i] - 1) -_b[i]) / _s[i]);
  }

  inline size_t length(const size_t i) const {
    SYMPHONY_INTERNAL_ASSERT(_e[i] - _b[i], "begin == end");
    return _e[i] - _b[i];
  }

  const std::array<size_t, Dims>& begin() const { return _b; }

  const std::array<size_t, Dims>& end() const { return _e; }

  const std::array<size_t, Dims>& stride() const { return _s; }

  size_t dims() const { return Dims; }
};

template<size_t Dims>
class range;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<>
class range<1> : public range_base<1>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");
public:
  range() : range_base<1>() { }

  range(const std::array<size_t, 1>& bb,
        const std::array<size_t, 1>& ee) : range_base<1>(bb, ee) { }

  range(const std::array<size_t, 1>& bb,
        const std::array<size_t, 1>& ee,
        const std::array<size_t, 1>& ss) : range_base<1>(bb, ee, ss) { }

  range(size_t b0, size_t e0, size_t s0) {
    SYMPHONY_API_ASSERT(((b0 <= e0) && (s0 > 0)), "Invalid range parameters");
    _b[0] = b0;
    _e[0] = e0;
    _s[0] = s0;
  }

  explicit range(size_t b0, size_t e0) : range(b0, e0, 1) { }

  explicit range(size_t e0) : range(0, e0) { }

  inline size_t size() const {
    return num_elems(0);
  }

  inline size_t linearized_distance() const {
    return length(0);
  }

  inline bool is_empty() const {
    return (!(_e[0] - _b[0]));
  }

  void increment_index(symphony::internal::index<1>& it) {
    check_bounds<1>::check(_b, _e, it.data());
    it[0] += _s[0];
  }

  size_t index_to_linear(const symphony::internal::index<1>& it) const {
    check_bounds<1>::check(_b, _e, it.data());
    check_stride<1>::check(_b, _s, it.data());

    return ((it[0] - _b[0]) / _s[0]);
  }

  symphony::internal::index<1> linear_to_index(size_t idx) const {
    index<1> it(idx * _s[0] + _b[0]);
    check_bounds<1>::check(_b, _e, it.data());
    return it;
  }

  void print() const {
    SYMPHONY_ALOG("(begin:end:stride) [%zu:%zu:%zu]", _b[0], _e[0], _s[0]);
  }
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<>
class range<2> : public range_base<2>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");
public:
  range() : range_base<2>() { }

  range(const std::array<size_t, 2>& bb,
        const std::array<size_t, 2>& ee) : range_base<2>(bb, ee) {

    for(auto& ss : _s) ss = 1;
  }

  range(const std::array<size_t, 2>& bb,
        const std::array<size_t, 2>& ee,
        const std::array<size_t, 2>& ss) : range_base<2>(bb, ee, ss) { }

  range(size_t b0, size_t e0, size_t s0, size_t b1, size_t e1, size_t s1) {
    SYMPHONY_API_ASSERT(((b0 <= e0) && (b1 <= e1) && (s0 > 0) && (s1 > 0)),
                      "Invalid range parameters");
    _b[0] = b0;
    _b[1] = b1;
    _e[0] = e0;
    _e[1] = e1;
    _s[0] = s0;
    _s[1] = s1;

    SYMPHONY_API_ASSERT(((num_elems(0) > 0) && (num_elems(1) > 0)),
                    "Invalid range parameters");
  }

  range(size_t b0, size_t e0, size_t b1, size_t e1) :
    range(b0, e0, 1, b1, e1, 1) { }

  range(size_t e0, size_t e1) :
    range(0, e0, 1, 0, e1, 1) { }

  void increment_index(symphony::internal::index<2>& it) {
    check_bounds<2>::check(_b, _e, it.data());
    if((it[0] + _s[0]) < _e[0]) {
      it[0] += _s[0];
      return;
    }

    if((it[1] + _s[1]) < _e[1]) {
      it[0] = _b[0];
      it[1] += _s[1];
      return;
    }
    it[0] += _s[0];
    it[1] += _s[1];
  }

  inline bool is_empty() const {
    return (!((_e[0] - _b[0]) && (_e[1] - _b[1])));
  }

  inline size_t size() const {
    return num_elems(0) * num_elems(1);
  }

  inline size_t linearized_distance() const {
    return length(0) * length(1);
  }

  size_t index_to_linear(const symphony::internal::index<2>& it) const {
    check_bounds<2>::check(_b, _e, it.data());
    check_stride<2>::check(_b, _s, it.data());
    return ((it[1] - _b[1])/_s[1]) * num_elems(0) +
      ((it[0] - _b[0])/_s[0]);

  }

  symphony::internal::index<2> linear_to_index(size_t idx) const {
    size_t j = idx / num_elems(0);
    size_t i = idx % num_elems(0);
    index<2> it(i * _s[0] + _b[0], j * _s[1] + _b[1]);

    check_bounds<2>::check(_b, _e, it.data());
    return it;
  }

  void print() const {
    SYMPHONY_ALOG("(begin:end:stride) [%zu:%zu:%zu], [%zu:%zu:%zu]",
              _b[0], _e[0], _s[0], _b[1], _e[1], _s[1]);
  }
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<>
class range<3> : public range_base<3>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");
public:
  range() : range_base<3>() { }

  range(const std::array<size_t, 3>& bb,
        const std::array<size_t, 3>& ee) : range_base<3>(bb, ee) {
    for(auto& ss: _s) ss = 1;
  }

  range(const std::array<size_t, 3>& bb,
        const std::array<size_t, 3>& ee,
        const std::array<size_t, 3>& ss) : range_base<3>(bb, ee, ss) { }

  range(size_t b0, size_t e0, size_t s0,
        size_t b1, size_t e1, size_t s1,
        size_t b2, size_t e2, size_t s2) {
    SYMPHONY_API_ASSERT(((b0 <= e0) && (b1 <= e1) && (b2 <= e2) &&
                     (s0 > 0) && (s1 > 0) && (s2 > 0)),
                       "Invalid range parameters");
    _b[0] = b0;
    _b[1] = b1;
    _b[2] = b2;
    _e[0] = e0;
    _e[1] = e1;
    _e[2] = e2;
    _s[0] = s0;
    _s[1] = s1;
    _s[2] = s2;

    SYMPHONY_API_ASSERT(((num_elems(0) > 0) && (num_elems(1) > 0) &&
                     (num_elems(2) > 0)), "Invalid range parameters");
  }

  range(size_t b0, size_t e0, size_t b1, size_t e1, size_t b2, size_t e2) :
    range(b0, e0, 1, b1, e1, 1, b2, e2, 1) { }

  range(size_t e0, size_t e1, size_t e2) :
    range(0, e0, 1, 0, e1, 1, 0, e2, 1) { }

  inline bool is_empty() const {
    return (!((_e[0] - _b[0]) && (_e[1] - _b[1])
              && (_e[2] - _b[2])));
  }

  inline size_t size() const {
    return num_elems(0) * num_elems(1) * num_elems(2);
  }

  inline size_t linearized_distance() const {
    return length(0) * length(1) * length(2);
  }

  void increment_index(symphony::internal::index<3>& it) {
    check_bounds<3>::check(_b, _e, it.data());
    if((it[0] + _s[0]) < _e[0]) {
      it[0] += _s[0];
      return;
    }

    if((it[1] + _s[1]) < _e[1]) {
      it[0] = _b[0];
      it[1] += _s[1];
      return;
    }

    if((it[2] + _s[2]) < _e[2]) {
      it[0] = _b[0];
      it[1] = _b[1];
      it[2] += _s[2];
      return;
    }
    it[0] += _s[0];
    it[1] += _s[1];
    it[2] += _s[2];
  }

  size_t index_to_linear(const symphony::internal::index<3>& it) const {
    check_bounds<3>::check(_b, _e, it.data());
    check_stride<3>::check(_b, _s, it.data());
    return (((it[2] - _b[2]) / _s[2]) * num_elems(0) * num_elems(1)+
            ((it[1] - _b[1]) / _s[1]) * num_elems(0) +
            ((it[0] - _b[0]) / _s[0]));
  }

  symphony::internal::index<3> linear_to_index(size_t idx) const {
    size_t k = idx / (num_elems(0) * num_elems(1));
    size_t j = (idx / num_elems(0)) % num_elems(1);
    size_t i = idx % num_elems(0);
    index<3> it(i * _s[0] + _b[0], j * _s[1] + _b[1], k * _s[2] + _b[2]);

    check_bounds<3>::check(_b, _e, it.data());
    return it;
  }

  void print() const {
    SYMPHONY_ALOG("(begin:end:stride) [%zu:%zu:%zu], [%zu:%zu:%zu], [%zu:%zu:%zu]",
              _b[0], _e[0], _s[0], _b[1], _e[1], _s[1], _b[2], _e[2], _s[2]);
  }
};
};

};
