// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/exceptions.hh>

#include <symphony/internal/storage/taskstorage.hh>

namespace symphony {

template<typename T>
class task_storage_ptr {
public:
  typedef T const* pointer_type;

  SYMPHONY_DELETE_METHOD(task_storage_ptr(task_storage_ptr const&));
  SYMPHONY_DELETE_METHOD(task_storage_ptr& operator=(task_storage_ptr const&));
#ifndef _MSC_VER

  SYMPHONY_DELETE_METHOD(task_storage_ptr& operator=
                     (task_storage_ptr const&) volatile);

#endif

  task_storage_ptr()
    : _key() {
    init_key();
  }

  explicit task_storage_ptr(T* const& ptr)
    : _key() {
    init_key();
    *this = ptr;
  }

  task_storage_ptr(T* const& ptr, void (*dtor)(T*))
    : _key() {
    init_key(dtor);
    *this = ptr;
  }

  T* get() const {
    return static_cast<T*>(task_get_specific(_key));
  }

  T* operator->() const {
    return get();
  }

  T& operator*() const {
    return *get();
  }

  task_storage_ptr& operator=(T* const& ptr) {
    task_set_specific(_key, ptr);
    return *this;
  }

  bool operator!() const {
    pointer_type t = get();
    return t == nullptr;
  }

  bool operator==(T* const& other) const {
    return get() == other;
  }

  bool operator!=(T* const& other) const {
    return get() != other;
  }

   operator pointer_type() const {
    return get();
  }

  explicit operator bool() const {
    return get() != nullptr;
  }

private:
  void init_key(void (*dtor)(T*) = nullptr) {
    int err = task_key_create(&_key, reinterpret_cast<void(*)(void*)>(dtor));
    if (err != 0)
      throw symphony::tls_exception("Unable to allocate task-local storage key.",
                                __FILE__, __LINE__, __FUNCTION__);
  }
private:
  internal::storage_key _key;
};

};
