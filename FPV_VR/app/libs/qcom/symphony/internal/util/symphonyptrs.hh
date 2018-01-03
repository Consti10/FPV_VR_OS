// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include<symphony/internal/log/log.hh>
#include<symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {
namespace testing {

  class SymphonyPtrSuite;

};

template<typename Object>
class symphony_shared_ptr;

template <typename Object>
void explicit_unref(Object* obj);

template <typename Object>
void explicit_ref(Object* obj);

typedef uint32_t ref_counter_type;

namespace symphonyptrs {

template<typename Object>
struct always_delete {
  static void release(Object* obj) {
    delete obj;
  }
};

struct default_logger {

  static void ref(void* o, ref_counter_type count) {
    log::fire_event<log::events::object_reffed>(o, count);
  }

  static void unref(void* o, ref_counter_type count) {
    log::fire_event<log::events::object_reffed>(o, count);
  }
};

enum class ref_policy {
  do_initial_ref,
  no_initial_ref
};

};

template<typename Object,
         class Logger = symphonyptrs::default_logger,
         class ReleaseManager = symphonyptrs::always_delete<Object>
         >
class ref_counted_object {
public:
  using size_type =  ref_counter_type;

  size_type use_count() const {
    return _num_refs.load();
  }

protected:

  explicit ref_counted_object(size_type initial_value)
    :_num_refs(initial_value) {
  }

  constexpr ref_counted_object()
    :_num_refs(0) {
  }

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

  ~ref_counted_object(){}

SYMPHONY_GCC_IGNORE_END("-Weffc++");

private:

  void unref() {

    auto new_num_refs = --_num_refs;
    Logger::unref(static_cast<Object*>(this), new_num_refs);

    if (new_num_refs == 0)
      ReleaseManager::release(static_cast<Object*>(this));
  }

  void ref() {
    auto new_num_refs = ++_num_refs;
    Logger::ref(static_cast<Object*>(this), new_num_refs);
  }

  friend class symphony_shared_ptr<Object>;
  template<typename Descendant>
  friend void explicit_unref(Descendant* obj);
  template<typename Descendant>
  friend void explicit_ref(Descendant* obj);

  std::atomic<size_type> _num_refs;
};

template <typename Object>
void explicit_unref(Object* obj) {
  obj->unref();
}

template <typename Object>
void explicit_ref(Object* obj) {
  obj->ref();
}

template<typename Object>
bool operator==(symphony_shared_ptr<Object> const&, symphony_shared_ptr<Object> const&);

template<typename Object>
bool operator!=(symphony_shared_ptr<Object> const&, symphony_shared_ptr<Object> const&);

template<typename Object>
class symphony_shared_ptr {
public:

  typedef Object type;
  typedef Object* pointer;
  typedef symphonyptrs::ref_policy ref_policy;

  constexpr symphony_shared_ptr()
    :_target(nullptr) {
  }

  constexpr  symphony_shared_ptr(std::nullptr_t)
    :_target(nullptr) {
  }

  symphony_shared_ptr(symphony_shared_ptr const& other)
    :_target(other._target) {
    if (_target != nullptr)
      _target->ref();
  }

  symphony_shared_ptr(symphony_shared_ptr&& other)
    : _target(other._target) {

    other._target = nullptr;
  }

  ~symphony_shared_ptr() {
    reset(nullptr);
  }

  symphony_shared_ptr& operator=(symphony_shared_ptr const& other) {
    if (other._target == _target)
      return *this;

    reset(other._target);
    return *this;
  }

  symphony_shared_ptr& operator=(std::nullptr_t) {
    if (_target  == nullptr)
      return *this;

    reset(nullptr);
    return *this;
  }

  symphony_shared_ptr& operator=(symphony_shared_ptr&& other) {
    if (other._target == _target) {
      return *this;
    }
    unref();

    _target = other._target;
    other._target = nullptr;

    return *this;
  }

  void swap(symphony_shared_ptr& other) {
    std::swap(_target, other._target);
  }

  Object* get_raw_ptr() const {
    return _target;
  }

  Object* reset_but_not_unref() {
    pointer t = _target;
    _target = nullptr;
    return t;
  }

  void reset() {
    reset(nullptr);
  }

  void reset(pointer ref) {
    unref();
    acquire(ref);
  }

  explicit operator bool() const  {
    return _target != nullptr;
  }

  Object& operator*() const {
    return *_target;
  }

  Object* operator->() const {
    return _target;
  }

  size_t use_count() const {
    if (_target != nullptr)
      return _target->use_count();
    return 0;
  }

  bool is_unique() const {
    return use_count() == 1;
  }

  explicit symphony_shared_ptr(pointer ref) :
    _target(ref) {
    if (_target != nullptr)
      _target->ref();
  }

  symphony_shared_ptr(pointer ref,  ref_policy policy)
    :_target(ref) {
    if ((_target != nullptr) && policy == ref_policy::do_initial_ref)
      _target->ref();
  }

private:

  void acquire(pointer other){
    _target = other;
    if (_target != nullptr)
      _target->ref();
  }

  void unref() {
    if (_target != nullptr)
      _target->unref();
  }

  friend bool operator==<>(symphony_shared_ptr<Object> const&,
                           symphony_shared_ptr<Object> const&);
  friend bool operator!=<>(symphony_shared_ptr<Object> const&,
                           symphony_shared_ptr<Object> const&);

  pointer _target;
};

template<typename Object>
bool operator==(symphony_shared_ptr<Object> const& a_ptr,
                symphony_shared_ptr<Object> const& b_ptr) {
  return a_ptr.get_raw_ptr() == b_ptr.get_raw_ptr();
}

template<typename Object>
bool operator==(symphony_shared_ptr<Object> const& ptr, std::nullptr_t) {
  return !ptr;
}

template<typename Object>
bool operator==(std::nullptr_t, symphony_shared_ptr<Object> const& ptr) {
  return !ptr;
}

template<typename Object>
bool operator!=(symphony_shared_ptr<Object> const& a_ptr,
                symphony_shared_ptr<Object> const& b_ptr) {
  return a_ptr.get_raw_ptr()!=b_ptr.get_raw_ptr();
}

template<typename Object>
bool operator!=(symphony_shared_ptr<Object> const& ptr, std::nullptr_t) {
  return static_cast<bool>(ptr);
}

template<typename Object>
bool operator!=(std::nullptr_t, symphony_shared_ptr<Object> const& ptr) {
  return static_cast<bool>(ptr);
}

template <typename Object>
Object* c_ptr(symphony_shared_ptr<Object>& t) {
  return static_cast<Object*>(t.get_raw_ptr());
}

template <typename Object>
Object* c_ptr(symphony_shared_ptr<Object>const& t) {
  return static_cast<Object*>(t.get_raw_ptr());
}

template<typename Object>
Object* c_ptr(Object* p) {
  return p;
}

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename Object>
class symphony_buffer_ptr : public std::shared_ptr<Object>
{

SYMPHONY_GCC_IGNORE_END("-Weffc++");

private:
  typedef typename Object::Type ElementType;
public:
  constexpr symphony_buffer_ptr() :  std::shared_ptr<Object>() { }

  template< class ManagedObject >
  explicit symphony_buffer_ptr( ManagedObject* ptr ) : std::shared_ptr<Object>(ptr) { }

  template< class ManagedObject, class Deleter >
  symphony_buffer_ptr( ManagedObject* ptr, Deleter d ) : std::shared_ptr<Object>(ptr, d) { }

  template< class ManagedObject, class Deleter, class Alloc >
  symphony_buffer_ptr( ManagedObject* ptr, Deleter d, Alloc alloc ) :
    std::shared_ptr<Object>(ptr, d, alloc) { }

  constexpr explicit symphony_buffer_ptr( std::nullptr_t ptr) : std::shared_ptr<Object>(ptr) { }

  template< class Deleter >
  symphony_buffer_ptr( std::nullptr_t ptr, Deleter d ) :
    std::shared_ptr<Object>( ptr, d) { }

  template< class Deleter, class Alloc >
  symphony_buffer_ptr( std::nullptr_t ptr, Deleter d, Alloc alloc ) :
    std::shared_ptr<Object>( ptr, d, alloc) { }

  template< class ManagedObject >
  symphony_buffer_ptr( const std::shared_ptr<ManagedObject>& r, Object *ptr ) :
    std::shared_ptr<Object>(r, ptr) { }

  explicit symphony_buffer_ptr( const std::shared_ptr<Object>& r ) : std::shared_ptr<Object>(r) { }

  template< class ManagedObject >
  explicit symphony_buffer_ptr( const std::shared_ptr<ManagedObject>& r ) : std::shared_ptr<Object>(r) { }

  explicit symphony_buffer_ptr( std::shared_ptr<Object>&& r ) : std::shared_ptr<Object>(r) { }

  template< class ManagedObject >
  explicit symphony_buffer_ptr( std::shared_ptr<ManagedObject>&& r ) : std::shared_ptr<Object>(r) { }

  template< class ManagedObject >
  explicit symphony_buffer_ptr( const std::weak_ptr<ManagedObject>& r ) :
    std::shared_ptr<Object>(r) { }

  ElementType& operator [] (size_t index)
  {
    Object& symphony_buffer = std::shared_ptr<Object>::operator * ();
    return symphony_buffer[index];
  }

  const ElementType& operator [] (size_t index) const
  {
    Object& symphony_buffer = std::shared_ptr<Object>::operator * ();
    return symphony_buffer[index];
  }

  Object& operator * () const { return std::shared_ptr<Object>::operator * (); }

  Object* operator -> () const { return std::shared_ptr<Object>::operator -> (); }
};

};
};
