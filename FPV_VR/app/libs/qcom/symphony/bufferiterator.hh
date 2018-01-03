// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/buffer/buffer.hh>
#include <symphony/internal/util/debug.hh>

namespace symphony {

template<typename T>
class buffer_const_iterator;

template<typename T>
class buffer_iterator {

  buffer_ptr<T>* _p_buffer_ptr;
  size_t         _index;

  buffer_iterator(buffer_ptr<T>* p_buffer_ptr, size_t index) :
    _p_buffer_ptr(p_buffer_ptr),
    _index(index)
  {}

  friend
  class buffer_ptr<T>;

  friend
  class buffer_const_iterator<T>;

public:

  T& operator*() const { return (*_p_buffer_ptr)[_index]; }

  T& operator[](size_t n) const {
    return (*_p_buffer_ptr)[_index + n];
  }

  buffer_iterator& operator++() {
    ++_index;
    return *this;
  }

  buffer_iterator& operator--() {
    --_index;
    return *this;
  }

  buffer_iterator operator++(int) {
    auto current = *this;
    ++(*this);
    return current;
  }

  buffer_iterator operator--(int) {
    auto current = *this;
    --(*this);
    return current;
  }

  buffer_iterator& operator+=(size_t offset) {
    _index += offset;
    return *this;
  }

  buffer_iterator& operator-=(size_t offset) {
    _index -= offset;
    return *this;
  }

  buffer_iterator operator+(size_t offset) {
    auto updated = *this;
    updated += offset;
    return updated;
  }

  buffer_iterator operator-(size_t offset) {
    auto updated = *this;
    updated -= offset;
    return updated;
  }

  int operator-(buffer_iterator const& it) {
    return static_cast<int>(_index) - static_cast<int>(it._index);
  }

  bool operator<(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(it._p_buffer_ptr != nullptr,
                         "buffer_iterator set to a null buffer");
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index < it._index;
  }

  bool operator>(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index > it._index;
  }

  bool operator<=(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index <= it._index;
  }

  bool operator>=(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index >= it._index;
  }

  bool operator==(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index == it._index;
  }

  bool operator!=(buffer_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index != it._index;
  }

  SYMPHONY_DEFAULT_METHOD(buffer_iterator(buffer_iterator const&));
  SYMPHONY_DEFAULT_METHOD(buffer_iterator& operator=(buffer_iterator const&));
  SYMPHONY_DEFAULT_METHOD(buffer_iterator(buffer_iterator&&));
  SYMPHONY_DEFAULT_METHOD(buffer_iterator& operator=(buffer_iterator&&));
};

template<typename T>
class buffer_const_iterator {

  buffer_ptr<T> const* _p_buffer_ptr;
  size_t               _index;

  buffer_const_iterator(buffer_ptr<T> const* p_buffer_ptr, size_t index) :
    _p_buffer_ptr(p_buffer_ptr),
    _index(index)
  {}

  friend
  class buffer_ptr<T>;

public:

   buffer_const_iterator(buffer_iterator<T> const& it) :
    _p_buffer_ptr(it._p_buffer_ptr),
    _index(it._index)
  {}

  T const& operator*() const { return (*_p_buffer_ptr)[_index]; }

  T const& operator[](size_t n) const {
    return (*_p_buffer_ptr)[_index + n];
  }

  buffer_const_iterator& operator++() {
    ++_index;
    return *this;
  }

  buffer_const_iterator& operator--() {
    --_index;
    return *this;
  }

  buffer_const_iterator operator++(int) {
    auto current = *this;
    ++(*this);
    return current;
  }

  buffer_const_iterator operator--(int) {
    auto current = *this;
    --(*this);
    return current;
  }

  buffer_const_iterator& operator+=(size_t offset) {
    _index += offset;
    return *this;
  }

  buffer_const_iterator& operator-=(size_t offset) {
    _index -= offset;
    return *this;
  }

  buffer_const_iterator operator+(size_t offset) {
    auto updated = *this;
    updated += offset;
    return updated;
  }

  buffer_const_iterator operator-(size_t offset) {
    auto updated = *this;
    updated -= offset;
    return updated;
  }

  int operator-(buffer_const_iterator const& it) {
    return static_cast<int>(_index) - static_cast<int>(it._index);
  }

  bool operator<(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(it._p_buffer_ptr != nullptr,
                         "buffer_const_iterator set to a null buffer");
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index < it._index;
  }

  bool operator>(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index > it._index;
  }

  bool operator<=(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index <= it._index;
  }

  bool operator>=(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index >= it._index;
  }

  bool operator==(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index == it._index;
  }

  bool operator!=(buffer_const_iterator const& it) const {
    SYMPHONY_INTERNAL_ASSERT(_p_buffer_ptr == it._p_buffer_ptr,
                         "buffer_const_iterator mismatch: %p %p",
                         _p_buffer_ptr,
                         it._p_buffer_ptr);
    return _index != it._index;
  }

  SYMPHONY_DEFAULT_METHOD(buffer_const_iterator(buffer_const_iterator const&));
  SYMPHONY_DEFAULT_METHOD(buffer_const_iterator& operator=(buffer_const_iterator const&));
  SYMPHONY_DEFAULT_METHOD(buffer_const_iterator(buffer_const_iterator&&));
  SYMPHONY_DEFAULT_METHOD(buffer_const_iterator& operator=(buffer_const_iterator&&));
};

};
