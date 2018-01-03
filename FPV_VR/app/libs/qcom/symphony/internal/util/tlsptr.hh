// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <memory>

#include <symphony/exceptions.hh>
#include <symphony/internal/compat/compat.h>

namespace symphony {

namespace internal {

namespace tls {
void error(std::string msg, const char* filename, int lineno,
           const char* funcname);
};

namespace storage {

template <typename T>
struct box {
  T* _ptr;
  explicit box(T* ptr) : _ptr(ptr) {};
  SYMPHONY_DELETE_METHOD(box(box const&));
  SYMPHONY_DELETE_METHOD(box& operator=(box const&));
  T* get() const {
    return _ptr;
  }
  box& operator=(T* const& ptr) {
    _ptr = ptr;
    return *this;
  }
};

template <typename T>
struct owner {
  T* _ptr;
  explicit owner(T* ptr) : _ptr(ptr) {};
  SYMPHONY_DELETE_METHOD(owner(owner const&));
  SYMPHONY_DELETE_METHOD(owner& operator=(owner const&));
  ~owner() {
    if (_ptr)
      delete _ptr;
  }
  T* get() const {
    return _ptr;
  }
  owner& operator=(T* const& ptr) {
    if (_ptr != nullptr)
      delete _ptr;
    _ptr = ptr;
    return *this;
  }
};

};

template<typename T, template <typename> class Box = storage::box>
class tlsptr {
public:
  typedef T* pointer_type;

  SYMPHONY_DELETE_METHOD(tlsptr(tlsptr const&));
  SYMPHONY_DELETE_METHOD(tlsptr& operator=(tlsptr const&));

  SYMPHONY_DELETE_METHOD(tlsptr& operator=(tlsptr const&) volatile);

  tlsptr(): _key(), _err() {
    initKey();
  }

  explicit tlsptr(T* const& ptr)
    : _key(), _err() {
    initKey();
    *this = ptr;
  }

  ~tlsptr() {
    if (_err != 0) {

      return;
    }
    if (int err = pthread_key_delete(_key)) {

      switch(err) {
      case EINVAL:
        throw symphony::tls_exception("The specified argument is not correct.",
                                  __FILE__, __LINE__, __FUNCTION__);
      case ENOENT:
        throw symphony::tls_exception("Specified TLS key is not allocated.",
                                  __FILE__, __LINE__, __FUNCTION__);
      default:
        throw symphony::tls_exception("Unknown error.", __FILE__,
                                  __LINE__, __FUNCTION__);
      }
    }
  }

  T* get() const {
    if (auto p = pthread_getspecific(_key)) {
      return static_cast<Box<T>*>(p)->get();
    }
    return pointer_type();
  }

  T* operator->() {
    return get();
  }

  T& operator*() {
    return *get();
  }

  tlsptr& operator=(T* const& ptr) {

    if (auto p = pthread_getspecific(_key)) {
      *static_cast<Box<T>*>(p) = ptr;
    } else {
      int rc = pthread_setspecific(_key, new Box<T>(ptr));
      if (rc != 0) {
        std::string msg =
          strprintf("Call to pthread_setspecific() failed [%d]:%s",
                    rc,
                    symphony_strerror(rc).c_str());
        tls::error(msg, __FILE__, __LINE__, __FUNCTION__);
      }
    }
    return *this;
  }

  bool operator!() {
    pointer_type t = get();
    return t==nullptr;
  }

  bool operator==(T* const& other) {
    return get()==other;
  }

  bool operator!=(T* const& other) {
    return get()!=other;
  }

   operator pointer_type() const {
    return get();
  }

  explicit operator bool() const {
    return get()!=nullptr;
  }

  const pthread_key_t& key() {
    return _key;
  }

  int get_error() const {
    return _err;
  }

private:
  static void delete_box(void* ptr) {
    auto boxptr = static_cast<Box<T>*>(ptr);
    if (boxptr)
      delete boxptr;
  }

  void initKey() {
    _err = pthread_key_create(&_key, delete_box);

  }
private:
  pthread_key_t _key;
  int _err;
};
};
};
