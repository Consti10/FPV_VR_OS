// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <memory>

#include <symphony/exceptions.hh>

#include <symphony/internal/storage/storagemap.hh>

namespace symphony {

template<template<class, class> class Scope, typename T, class Allocator>
class scoped_storage_ptr {

public:
  typedef Scope<T,Allocator> scope_type;
  typedef T const* pointer_type;

  SYMPHONY_DELETE_METHOD(scoped_storage_ptr(scoped_storage_ptr const&));
  SYMPHONY_DELETE_METHOD(scoped_storage_ptr& operator=(scoped_storage_ptr const&));

#ifndef _MSC_VER

  SYMPHONY_DELETE_METHOD(scoped_storage_ptr& operator=
                     (scoped_storage_ptr const&) volatile);

#endif

  SYMPHONY_DELETE_METHOD(scoped_storage_ptr& operator=(T* const&));

  scoped_storage_ptr()
    : _key() {
    init_key();
  }
  virtual ~scoped_storage_ptr() {}

  T* get() const {
    auto pvalue = static_cast<T*>(scope_type::get_specific(_key));
    if (!pvalue) {
      pvalue = _alloc.allocate(1);
      _alloc.construct(pvalue);
      scope_type::set_specific(_key, pvalue);
    }
    return pvalue;
  }

  T* operator->() const {
    return get();
  }

  T& operator*() const {
    return *get();
  }

  bool operator!() const {
#ifdef SYMPHONY_CHECK_INTERNAL
    pointer_type t = get();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "invalid value for scoped storage %p",
                         this);
    return t == nullptr;
#else
    return false;
#endif
  }

  bool operator==(T* const& other) {
    return get() == other;
  }

  bool operator!=(T* const& other) {
    return get() != other;
  }

   operator pointer_type() const {
    return get();
  }

  explicit operator bool() const {
#ifdef SYMPHONY_CHECK_INTERNAL
    return get() != nullptr;
#else
    return true;
#endif
  }

private:
  static void maybe_invoke_dtor(void* ptr) {
    if (!ptr)
      return;
    auto obj = static_cast<T*>(ptr);
    _alloc.destroy(obj);
    _alloc.deallocate(obj, 1);
  }

  void init_key() {
    int err = scope_type::key_create(&_key, &maybe_invoke_dtor);
    if (err != 0)
      throw symphony::tls_exception("Unable to allocate storage key.",
                                __FILE__, __LINE__, __FUNCTION__);
  }
private:
  internal::storage_key _key;
  static Allocator _alloc;
};

template<template<class,class> class S, typename T, typename A>
A scoped_storage_ptr<S, T, A>::_alloc;

};
