// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <type_traits>

#include <symphony/internal/compat/compat.h>

namespace symphony {

namespace internal {

template<class InputIterator,
         bool = std::is_integral<InputIterator>::value>
class distance_helper
{
public:
  typedef typename std::iterator_traits<InputIterator>::difference_type
    _result_type;

  static _result_type s_distance(InputIterator first, InputIterator last)
  {
    return std::distance(first, last);
  }
};

template <typename IntegralType>
class distance_helper<IntegralType,true>
{
public:
  typedef IntegralType _result_type;

  static _result_type s_distance(IntegralType first, IntegralType last)
  {
    return last - first;
  }
};

template<class InputIterator>
auto distance(InputIterator first, InputIterator last) ->
  typename distance_helper<InputIterator>::_result_type
{
  return distance_helper<InputIterator>::s_distance(first, last);
}
};

};
