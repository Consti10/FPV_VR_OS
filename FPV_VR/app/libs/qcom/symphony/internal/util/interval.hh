// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace symphony {
namespace internal {

template<class T, class Enable = void>
struct interval_traits {
  using difference_type = typename std::iterator_traits<T>::difference_type;
  static difference_type distance(T a, T b) {
    return std::distance(a, b);
  }
};

template<class T>
struct interval_traits<T, typename std::enable_if<
                            std::is_integral<T>::value>::type> {
  using difference_type = std::ptrdiff_t;
  static difference_type distance(T a,T b) {
    return b - a;
  }
};

template<>
struct interval_traits<const void *> {
  using difference_type = std::ptrdiff_t;
  static difference_type distance(const void* a, const void* b) {
    return static_cast<const char*>(b) - static_cast<const char*>(a);
  }
};

template<>
struct interval_traits<void *> {
  using difference_type = std::ptrdiff_t;
  static difference_type distance(const void* a, const void* b) {
    return static_cast<const char*>(b) - static_cast<const char*>(a);
  }
};

template<class T>
class interval {
  T _begin;
  T _end;
public:

  interval(T i_begin, T i_end) : _begin(i_begin),
                                 _end(i_end) {
    SYMPHONY_INTERNAL_ASSERT(_begin <= _end, "illegal interval");
  }

  interval(interval const& other) : _begin(other.begin()),
                                    _end(other.end()) {}

  auto length() const
    -> typename interval_traits<T>::difference_type {
    return interval_traits<T>::distance(_begin, _end);
  }

  T begin() const {
    return _begin;
  }

  T end() const {
    return _end;
  }

  bool operator==(interval const& other) const {
    return begin() == other.begin() &&
      end() == other.end();
  }

  bool operator<(interval const& other) const {
    return begin() < other.begin();
  }
};

template<class T>
interval<T> make_interval(T begin, T end)
{
  return interval<T>(begin, end);
}

template<class T>
interval<T> make_length_interval(T begin, std::ptrdiff_t length)
{
  return interval<T>(begin, begin + length);
}

template<class T>
std::ostream&
operator<<(std::ostream& stream, interval<T> const& interval)
{
  stream << "[" << interval.begin() << "," << interval.end() << ")";
  return stream;
}

};
};
