// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/util/index.hh>

namespace symphony {

#ifdef ONLY_FOR_DOXYGEN

#error The compiler should not see these definitions

template<size_t Dims>
class index : public internal::index<Dims>
{
public:

  explicit index(const internal::index<Dims>& rhs) : internal::index<Dims>(rhs) { }

  index<Dims>& operator = (const index<Dims>& rhs) {
    internal::index<Dims>::operator=(rhs);
    return (*this);
  }

  index<Dims>& operator += (const index<Dims>& rhs) {
    internal::index<Dims>::operator+=(rhs);
    return (*this);
  }

  index<Dims>& operator -= (const index<Dims>& rhs) {
    internal::index<Dims>::operator-=(rhs);
    return (*this);
  }

  index<Dims> operator - (const index<Dims>& rhs) {
    return index<Dims>(internal::index<Dims>::operator-(rhs));
  }

  index<Dims> operator + (const index<Dims>& rhs) {
    return index<Dims>(internal::index<Dims>::operator+(rhs));
  }

  bool operator == (const index_base<Dims>& rhs) const;

  bool operator != (const index_base<Dims>& rhs) const;

  bool operator < (const index_base<Dims>& rhs) const;

  bool operator <= (const index_base<Dims>& rhs) const;

  bool operator > (const index_base<Dims>& rhs) const;

  bool operator >= (const index_base<Dims>& rhs) const;

  size_t& operator [] (size_t i);

  const size_t& operator [] (size_t i) const;

  const std::array<size_t, Dims>& data() const;
};

template<>
class index<1>
{
public:

  index();

  explicit index(const std::array<size_t, 1>& rhs);

  explicit index(size_t i);
};

template<>
class index<2>
{
public:

  index();

  explicit index(const std::array<size_t, 2>& rhs);

  index(size_t i, size_t j);
};

template<>
class index<3>
{
public:

  index();

  explicit index(const std::array<size_t, 3>& rhs);

  index(size_t i, size_t j, size_t k);
};

#endif

template<size_t Dims>
using index = internal::index<Dims>;

};
