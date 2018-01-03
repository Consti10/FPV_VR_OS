// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <iterator>
#include <set>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/interval.hh>

namespace symphony {
namespace internal {

struct interval_set_base {};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<class T, template<class> class Interval = interval>
class interval_set : public interval_set_base {
private:
  using set_type = typename std::set<Interval<T>>;
  set_type _set;

public:
  using interval_type = Interval<T>;
  using element_type = T;
  using iterator = typename set_type::iterator;
  using const_iterator = typename set_type::const_iterator;

  interval_set() : _set() {}

  iterator end() {
    return _set.end();
  }

  const_iterator end() const {
    return _set.end();
  }

  const_iterator cend() const {
    return end();
  }

  iterator begin() {
    return _set.begin();
  }

  const_iterator begin() const {
    return _set.begin();
  }

  const_iterator cbegin() const {
    return begin();
  }

public:

  iterator find(T const& point) {
    iterator it = _set.lower_bound(interval_type(point, point));
    if (it != end()) {
      if (it->begin() <= point && point < it->end())
        return it;
    }

    if (it == begin()) {

      return end();
    }

    --it;
    SYMPHONY_INTERNAL_ASSERT(it->begin() <= point, "invalid lower bound");
    if (point < it->end())
      return it;

    return end();
  }

  const_iterator find(T const& point) const {
    const_iterator it = _set.lower_bound(interval_type(point, point));
    if (it != end()) {
      if (it->begin() <= point && point < it->end())
        return it;
    }

    if (it == begin()) {

      return end();
    }

    --it;
    SYMPHONY_INTERNAL_ASSERT(it->begin() <= point, "invalid lower bound");
    if (point < it->end())
      return it;

    return end();
  }

  void insert(interval_type const& interval) {
    _set.insert(interval);
  }

  size_t erase(interval_type const&& interval) {
    return _set.erase(std::forward<interval_type const>(interval));
  }

};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
