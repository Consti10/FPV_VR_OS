// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/util/range.hh>

namespace symphony {

#ifdef ONLY_FOR_DOXYGEN

#error The compiler should not see these definitions

template<size_t Dims>
class range
{
public:

  inline size_t size() const;

  inline size_t linearized_distance() const;

  size_t dims() const;

  size_t begin(const size_t i) const;

  size_t end(const size_t i) const;

  size_t stride(const size_t i) const;

  const std::array<size_t, Dims>& begin();

  const std::array<size_t, Dims>& end();

  const std::array<size_t, Dims>& stride() const;

  size_t num_elems(const size_t i) const;

   size_t length(const size_t i) const;
};

template<>
class range<1> : public internal::range<1>
{
public:

  range() : internal::range<1>() { }

  range(size_t b0, size_t e0) : internal::range<1>(b0, e0) { }

  explicit range(size_t e0) : internal::range<1>(e0) { }

  range(size_t b0, size_t e0, size_t s0) : internal::range<1>(b0, e0, s0) { }

  size_t index_to_linear(const symphony::index<1>& it) const;

  symphony::index<1> linear_to_index(size_t idx) const;
};

template<>
class range<2> : public internal::range<2>
{
public:

  range() : range<2>() { }

  range(size_t b0, size_t e0, size_t b1, size_t e1) :
    internal::range<2>(b0, e0, b1, e1) { }

  range(size_t e0, size_t e1) : internal::range<2>(e0, e1) { }

  range(size_t b0, size_t e0, size_t s0, size_t b1, size_t e1, size_t s1) :
    internal::range<2>(b0, e0, s0, b1, e1, s1) { }

  size_t index_to_linear(const symphony::index<2>& it) const;

  symphony::index<2> linear_to_index(size_t idx) const;

};

template<>
class range<3> : public internal::range<3>
{
public:

  range() : range<3>() { }

  range(size_t b0, size_t e0, size_t b1, size_t e1, size_t b2, size_t e2) :
    internal::range<3>(b0, e0, b1, e1, b2, e2) { }

  range(size_t e0, size_t e1, size_t e2) : internal::range<3>(e0, e1, e2) { }

  range(size_t b0, size_t e0, size_t s0,
        size_t b1, size_t e1, size_t s1,
        size_t b2, size_t e2, size_t s2) :
    internal::range<3>(b0, e0, s0, b1, e1, s1, b2, e2, s2) { }

  size_t index_to_linear(const symphony::index<3>& it) const;

  symphony::index<3> linear_to_index(size_t idx) const;
};
#endif

template<size_t Dims>
using range = internal::range<Dims>;

};
