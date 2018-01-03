// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/exceptions.hh>
#include <symphony/task.hh>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/symphonyptrs.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {

namespace internal {
namespace testing {
  class task_tests;
};
};

template<typename ...Stuff> class task_ptr;

template<typename Fn> class cpu_body;

template<typename ReturnType, typename... Args>
::symphony::task_ptr<ReturnType> create_value_task(Args&& ...args);

template<>
class task_ptr<> {

  friend ::symphony::internal::task* ::symphony::internal::c_ptr(::symphony::task_ptr<>& t);
  friend ::symphony::internal::task* ::symphony::internal::c_ptr(::symphony::task_ptr<> const& t);
  friend class ::symphony::internal::testing::task_tests;

public:

  using task_type = task<>;

  task_ptr() :
    _shared_ptr(nullptr) {
  }

   task_ptr(std::nullptr_t) :
    _shared_ptr(nullptr) {
  }

  task_ptr(task_ptr const& other) :
    _shared_ptr(other._shared_ptr) {
  }

  task_ptr(task_ptr&& other) :
    _shared_ptr(std::move(other._shared_ptr)) {
  }

  task_ptr& operator=(task_ptr const& other) {
    _shared_ptr = other._shared_ptr;
    return *this;
  }

  task_ptr& operator=(std::nullptr_t) {
    _shared_ptr = nullptr;
    return *this;
  }

  task_ptr& operator=(task_ptr&& other) {
    _shared_ptr = (std::move(other._shared_ptr));
    return *this;
  }

  void swap(task_ptr& other) {
    std::swap(_shared_ptr, other._shared_ptr);
  }

  task_type* operator->() const {
    auto t = get_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "null task pointer.");
    return t->get_facade<task_type>();
  }

  task_type* get() const {
    auto t = get_raw_ptr();
    if (t == nullptr)
      return nullptr;
    return t->get_facade<task_type>();
  }

  void reset() {
    _shared_ptr.reset();
  }

  explicit operator bool() const {
    return _shared_ptr != nullptr;
  }

  size_t use_count() const {
    return _shared_ptr.use_count();
  }

  bool unique() const {
    return _shared_ptr.use_count() == 1;
  }

  ~task_ptr() {}

protected:

  explicit task_ptr(::symphony::internal::task* t) :
    _shared_ptr(t) {
  }

  task_ptr(::symphony::internal::task* t, ::symphony::internal::task_shared_ptr::ref_policy policy) :
    _shared_ptr(t, policy) {
  }

  ::symphony::internal::task* get_raw_ptr() const {
    return ::symphony::internal::c_ptr(_shared_ptr);
  }

private:
  ::symphony::internal::symphony_shared_ptr<::symphony::internal::task> _shared_ptr;

  static_assert(sizeof(task_type) == sizeof(::symphony::internal::task::self_ptr),
                "Can't allocate task.");
};

template<>
class task_ptr<void> : public task_ptr<> {

  using parent = task_ptr<>;

 public:

  using task_type = task<void>;

  task_ptr() :
    parent() {
  }

   task_ptr(std::nullptr_t) :
    parent(nullptr) {
  }

  task_ptr(task_ptr<void> const& other) :
    parent(other) {
  }

  task_ptr(task_ptr<void>&& other) :
    parent(std::move(other)) {
  }

  task_ptr& operator=(task_ptr<void> const& other) {
    parent::operator=(other);
    return *this;
  }

  task_ptr& operator=(task_ptr<void>&& other) {
    parent::operator=(std::move(other));
    return *this;
  }

  ~task_ptr() {

  }

  void swap(task_ptr<void>& other) {
    parent::swap(other);
  }

  task_type* operator->() const {
    auto t = get_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "null task pointer.");
    return t->get_facade<task_type>();
  }

  task_type* get() const {
    auto t = get_raw_ptr();
    if (t == nullptr)
      return nullptr;
    return t->get_facade<task_type>();
  }

  protected:

  task_ptr(::symphony::internal::task* t, ::symphony::internal::task_shared_ptr::ref_policy policy) :
    parent(t, policy) {
  }

};

template<typename ReturnType>
class task_ptr<ReturnType> : public task_ptr<> {

  static_assert(std::is_reference<ReturnType>::value == false,
                "This version of Qualcomm Technologies Inc. Symphony does not support references as return types of tasks. "
                "Check the Qualcomm Technologies Inc. Symphony manual.");

  using parent = task_ptr<>;

 public:

  using return_type = ReturnType;

  using task_type = task<return_type>;

  task_ptr():
    parent(nullptr) {
  }

   task_ptr(std::nullptr_t)
    :parent(nullptr) {
  }

  task_ptr(task_ptr<return_type> const& other) :
    parent(other) {
  }

  task_ptr(task_ptr<return_type>&& other) :
    parent(std::move(other)) {
  }

  task_ptr& operator=(task_ptr<return_type> const& other) {
    parent::operator=(other);
    return *this;
  }

  task_ptr& operator=(task_ptr<return_type>&& other) {
    parent::operator=(std::move(other));
    return *this;
  }

  template<typename T> task_ptr& operator+=(T&& op);

  template<typename T> task_ptr& operator-=(T&& op);

  template<typename T> task_ptr& operator*=(T&& op);

  template<typename T> task_ptr& operator/=(T&& op);

  template<typename T> task_ptr& operator%=(T&& op);

  template<typename T> task_ptr& operator&=(T&& op);

  template<typename T> task_ptr& operator|=(T&& op);

  template<typename T> task_ptr& operator^=(T&& op);

  ~task_ptr() {
  }

  void swap(task_ptr<return_type>& other) {
    parent::swap(other);
  }

  task_type* operator->() const {
    auto t = get_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "null task pointer.");
    return t->template get_facade<task_type>();
  }

  task_type* get() const {
    auto t = get_raw_ptr();
    if (t == nullptr)
      return nullptr;
    return t->template get_facade<task_type>();
  }

protected:

  task_ptr<return_type>(::symphony::internal::task* t, ::symphony::internal::task_shared_ptr::ref_policy policy) :
     parent(t, policy) {
   }

  template<typename R, typename... As>
  friend ::symphony::task_ptr<R> create_value_task(As&& ...args);

};

template<typename ReturnType, typename ...Args>
class task_ptr<ReturnType(Args...)> : public task_ptr<ReturnType> {

  using parent = task_ptr<ReturnType>;

public:

  using task_type = task<ReturnType(Args...)>;

  using size_type = typename task_type::size_type;

  using return_type = typename task_type::return_type;

  using args_tuple = typename task_type::args_tuple;

  static SYMPHONY_CONSTEXPR_CONST size_type arity = task_type::arity;

  template <size_type ArgIndex>
  struct argument {
    static_assert(ArgIndex < sizeof...(Args), "Out of range argument indexing.");
    using type = typename std::tuple_element<ArgIndex, args_tuple>::type;
  };

  task_ptr():
    parent() {
  }

   task_ptr(std::nullptr_t)
    :parent(nullptr) {
  }

  task_ptr(task_ptr<ReturnType(Args...)> const& other) :
    parent(other) {
  }

  task_ptr(task_ptr<ReturnType(Args...)>&& other) :
    parent(std::move(other)) {
  }

  task_ptr& operator=(task_ptr<ReturnType(Args...)> const& other) {
    parent::operator=(other);
    return *this;
  }

  task_ptr& operator=(task_ptr<ReturnType(Args...)>&& other) {
    parent::operator=(std::move(other));
    return *this;
  }

  ~task_ptr() {

  }

  void swap(task_ptr<ReturnType(Args...)>& other) {
    parent::swap(other);
  }

  task_type* operator->() const {
    auto t = task_ptr<>::get_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "null task pointer.");
    return t->template get_facade<task_type>();
  }

  task_type* get() const {
    auto t = task_ptr<>::get_raw_ptr();
    if (t == nullptr)
      return nullptr;
    return t->template get_facade<task_type>();
  }

  task_ptr(::symphony::internal::task* t, ::symphony::internal::task_shared_ptr::ref_policy policy) :
    parent(t, policy) {
  }

protected:

  static_assert(arity < ::symphony::internal::set_arg_tracker::max_arity::value,
                "Task has too many arguments. Refer to the Qualcomm Technologies Inc. Symphony manual.");

  template<class Fn>
  friend class cpu_body;

};

template<typename ReturnType, typename ...Args>
SYMPHONY_CONSTEXPR_CONST typename task_ptr<ReturnType(Args...)>::size_type
task_ptr<ReturnType(Args...)>::arity;

inline
bool operator==(task_ptr<> const& t, std::nullptr_t)  {
  return !t;
}

inline
bool operator==(std::nullptr_t, task_ptr<> const& t)  {
  return !t;
}

inline
bool operator!=(::symphony::task_ptr<> const& t, std::nullptr_t)  {
  return static_cast<bool>(t);
}

inline
bool operator!=(std::nullptr_t, ::symphony::task_ptr<> const& t)  {
  return static_cast<bool>(t);
}

inline
bool operator==(::symphony::task_ptr<> const& a,
                ::symphony::task_ptr<> const& b)  {
  return symphony::internal::c_ptr(a) == symphony::internal::c_ptr(b);
}

inline
bool operator!=(::symphony::task_ptr<> const& a,
                ::symphony::task_ptr<> const& b)  {
  return !(a==b);
}

inline
::symphony::task_ptr<>& operator>>(::symphony::task_ptr<>& pred,
                               ::symphony::task_ptr<>& succ) {
  pred->then(succ);
  return succ;
}

inline
void finish_after(::symphony::task<>* task)
{
  SYMPHONY_API_ASSERT(task != nullptr, "null task_ptr");
  auto t_ptr = internal::c_ptr(task);
  auto t = internal::current_task();
  SYMPHONY_API_THROW(t != nullptr,
      "finish_after must be called from within a task");
  SYMPHONY_API_THROW(!t->is_pfor(),
      "finish_after cannot be called from within a pfor_each");
  t->finish_after(t_ptr);
}

inline
void finish_after(::symphony::task_ptr<> const& task)
{
  auto t_ptr = internal::c_ptr(task);
  SYMPHONY_API_ASSERT(t_ptr != nullptr, "null task_ptr");
  auto t = internal::current_task();
  SYMPHONY_API_THROW(t != nullptr,
      "finish_after must be called from within a task");
  SYMPHONY_API_THROW(!t->is_pfor(),
      "finish_after cannot be called from within a pfor_each");
  t->finish_after(t_ptr);
}

namespace internal {

inline
::symphony::internal::task* c_ptr(::symphony::task_ptr<>& t) {
  return t.get_raw_ptr();
}

inline
::symphony::internal::task* c_ptr(::symphony::task_ptr<> const& t) {
  return t.get_raw_ptr();
}
};

#ifdef ONLY_FOR_DOXYGEN

template<typename T>
inline ::symphony::task_ptr<typename ::symphony::task_ptr<T>::return_type>
operator-(const ::symphony::task_ptr<T>& t);

template<typename T>
inline ::symphony::task_ptr<typename ::symphony::task_ptr<T>::return_type>
operator+(const ::symphony::task_ptr<T>& t);

template<typename T>
inline ::symphony::task_ptr<typename ::symphony::task_ptr<T>::return_type>
operator ~(const ::symphony::task_ptr<T>& t);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 +
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator+(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 +
                                 std::declval<T2>())>
operator+(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 +
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator+(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 -
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator-(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 -
                                 std::declval<T2>())>
operator-(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 -
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator-(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 *
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator*(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 *
                                 std::declval<T2>())>
operator*(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 *
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator*(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 /
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator/(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 /
                                 std::declval<T2>())>
operator/(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 /
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator/(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 %
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator%(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 %
                                 std::declval<T2>())>
operator%(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 %
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator%(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 &
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator&(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 &
                                 std::declval<T2>())>
operator&(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 &
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator&(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 ^
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator^(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 ^
                                 std::declval<T2>())>
operator^(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 ^
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator^(T1&& op1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 |
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator|(const ::symphony::task_ptr<T1>& t1, const ::symphony::task_ptr<T2>& t2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()
                                 |
                                 std::declval<T2>())>
operator|(const ::symphony::task_ptr<T1>& t1, T2&& op2);

template<typename T1, typename T2>
inline ::symphony::task_ptr<decltype(std::declval<T1>()
                                 |
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>
operator|(T1&& op1, const ::symphony::task_ptr<T2>& t2);
#endif

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
