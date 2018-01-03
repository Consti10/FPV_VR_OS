// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <iterator>
#include <map>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/interval.hh>

namespace symphony {
namespace internal {

struct interval_map_base {
  virtual ~interval_map_base() {};
};

template<class K, class V, template<class> class Interval = interval>
class interval_map : public interval_map_base {
private:
  using map_type = typename std::map<Interval<K>,V>;
  map_type _map;

public:
  using interval_type = Interval<K>;
  using key_type = K;
  using mapped_type = V;
  using value_type = typename map_type::value_type;
  using iterator = typename map_type::iterator ;
  using const_iterator = typename map_type::const_iterator;

  interval_map() : _map() {}

  iterator end() {
    return _map.end();
  }

  size_t size() {
    return _map.size();
  }

  const_iterator end() const {
    return _map.end();
  }

  const_iterator cend() const {
    return end();
  }

  iterator begin() {
    return _map.begin();
  }

  const_iterator begin() const {
    return _map.begin();
  }

  const_iterator cbegin() const {
    return begin();
  }

  iterator find(K const& point) {
    iterator it = _map.lower_bound(interval_type(point, point));
    if (it != end()) {
      if (it->first.begin() <= point && point < it->first.end())
        return it;
    }

    if (it == begin()) {

      return end();
    }

    --it;
    SYMPHONY_INTERNAL_ASSERT(it->first.begin() <= point, "invalid lower bound");
    if (point < it->first.end())
      return it;

    return end();
  }

  const_iterator find(K const& point) const {
    const_iterator it = _map.lower_bound(interval_type(point, point));
    if (it != end()) {
      if (it->first.begin() <= point && point < it->first.end())
        return it;
    }

    if (it == begin()) {

      return end();
    }

    --it;
    SYMPHONY_INTERNAL_ASSERT(it->first.begin() <= point, "invalid lower bound");
    if (point < it->first.end())
      return it;

    return end();
  }

  mapped_type& operator[](interval_type const& interval) {
    return _map[interval];
  }

  void insert(value_type&& value) {
    _map.insert(std::forward<value_type>(value));
  }

  size_t erase(interval_type const&& interval) {
    return _map.erase(std::forward<interval_type const>(interval));
  }

};

};
};
