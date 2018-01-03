// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>
#include <typeinfo>
#include <utility>

#include <symphony/internal/task/cputask.hh>
#include <symphony/internal/util/debug.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {

template<typename ...Stuff> class task;
template<typename ...Stuff> class task_ptr;

namespace internal {

::symphony::internal::task* c_ptr(::symphony::task_ptr<>& t);
::symphony::internal::task* c_ptr(::symphony::task_ptr<> const& t);
::symphony::internal::task* c_ptr(::symphony::task<>*);

template<typename R, typename... As>
cputask_arg_layer<R(As...)>* c_ptr2(::symphony::task<R(As...)>& t);

};

template<>
class task<> {

  friend ::symphony::internal::task* ::symphony::internal::c_ptr(::symphony::task<>* t);

protected:

  using internal_raw_task_ptr = ::symphony::internal::task*;

public:

  using size_type = ::symphony::internal::task::size_type;

  void launch() {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    get_raw_ptr()->launch(nullptr, nullptr);
  }

  void wait_for() {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    get_raw_ptr()->wait();
  }

  void finish_after() {
    auto p = get_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(p != nullptr, "Unexpected null task<>.");
    auto t = ::symphony::internal::current_task();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr,
        "finish_after must be called from within a task");
    SYMPHONY_INTERNAL_ASSERT(!t->is_pfor(),
        "finish_after cannot be called from within a pfor_each");
    t->finish_after(p);
  }

  task_ptr<>& then(task_ptr<>& succ) {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task.");

    auto succ_ptr = ::symphony::internal::c_ptr(succ);
    SYMPHONY_API_ASSERT(succ_ptr != nullptr, "null task_ptr<>");

    get_raw_ptr()->add_control_dependency(succ_ptr);
    return succ;
  }

  task<>* then(task<>* succ) {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task.");

    auto succ_ptr = ::symphony::internal::c_ptr(succ);
    SYMPHONY_API_ASSERT(succ_ptr != nullptr, "null task_ptr<>");

    get_raw_ptr()->add_control_dependency(succ_ptr);
    return succ;
  }

  void cancel() {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    get_raw_ptr()->cancel();
  }

  bool canceled() const {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    return get_raw_ptr()->is_canceled();
  }

  bool is_bound() const {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    return get_raw_ptr()->is_bound();
  }

  std::string to_string() const {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task<>.");
    return get_raw_ptr()->to_string();
  }

protected:

  internal_raw_task_ptr get_raw_ptr() const {
    return _ptr;
  }

  internal_raw_task_ptr _ptr;

  SYMPHONY_DELETE_METHOD(task());
  SYMPHONY_DELETE_METHOD(task(task const&));
  SYMPHONY_DELETE_METHOD(task(task&&));
  SYMPHONY_DELETE_METHOD(task& operator=(task const&));
  SYMPHONY_DELETE_METHOD(task& operator=(task&&));

};

template<>
class task<void> : public task<> {

protected:

  ~task() { }

  SYMPHONY_DELETE_METHOD(task());
  SYMPHONY_DELETE_METHOD(task(task const&));
  SYMPHONY_DELETE_METHOD(task(task&&));
  SYMPHONY_DELETE_METHOD(task& operator=(task const&));
  SYMPHONY_DELETE_METHOD(task& operator=(task&&));

};

template<typename ReturnType>
class task<ReturnType> : public task<> {

public:

  using return_type = ReturnType;

  return_type const& copy_value() {
    auto t = get_typed_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "Unexpected null task<ReturnType>");
    t->wait();
    return t->get_retval();
  }

  return_type&& move_value() {
    auto t = get_typed_raw_ptr();
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "Unexpected null task<ReturnType>");
    t->wait();
    return t->move_retval();
  }

private:

  using raw_ptr = symphony::internal::cputask_return_layer<return_type>*;

  raw_ptr get_typed_raw_ptr() const {
    SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null task*");
    return static_cast<raw_ptr>(get_raw_ptr());
  }

  ~task() { }

  SYMPHONY_DELETE_METHOD(task());
  SYMPHONY_DELETE_METHOD(task(task const&));
  SYMPHONY_DELETE_METHOD(task(task&&));
  SYMPHONY_DELETE_METHOD(task& operator=(task const&));
  SYMPHONY_DELETE_METHOD(task& operator=(task&&));
};

template<typename ReturnType, typename ...Args>
class task<ReturnType(Args...)> : public task<ReturnType> {

public:

  using size_type = task<>::size_type;

  using return_type = ReturnType;

  using args_tuple = std::tuple<Args...>;

  static SYMPHONY_CONSTEXPR_CONST size_type arity = sizeof...(Args);

  template <size_type ArgIndex>
  struct argument {
    static_assert(ArgIndex < sizeof...(Args), "Out of range argument indexing.");
    using type = typename std::tuple_element<ArgIndex, args_tuple>::type;
  };

  template<typename... Arguments>
  void bind_all(Arguments&&... args) {
    static_assert(sizeof...(Arguments) == sizeof...(Args),
                  "Invalide number of arguments");

    auto t = get_typed_raw_ptr();
    t->bind_all(std::forward<Arguments>(args)...);
  }

  template<typename... Arguments>
  void launch(Arguments&&... args) {

    using should_bind = typename std::conditional<
                          sizeof...(Arguments) != 0,
                          std::true_type,
                          std::false_type>::type;

    launch_impl(should_bind(), std::forward<Arguments>(args)...);
  }

private:

  using raw_ptr = symphony::internal::cputask_arg_layer<return_type(Args...)>*;

  raw_ptr get_typed_raw_ptr() const {
    SYMPHONY_API_ASSERT(task<>::get_raw_ptr(),  "Unexpected null ptr");
    return static_cast<raw_ptr>(task<>::get_raw_ptr());
  }

  template<typename... Arguments>
  void launch_impl(std::true_type, Arguments&&... args) {
    bind_all(std::forward<Arguments>(args)...);
    task<>::launch();
  }

  template<typename... Arguments>
  void launch_impl(std::false_type, Arguments&&...) {
    task<>::launch();
  }

  ~task() { }

  SYMPHONY_DELETE_METHOD(task());
  SYMPHONY_DELETE_METHOD(task(task const&));
  SYMPHONY_DELETE_METHOD(task(task&&));
  SYMPHONY_DELETE_METHOD(task& operator=(task const&));
  SYMPHONY_DELETE_METHOD(task& operator=(task&&));

  template<typename R, typename... As>
  friend
  ::symphony::internal::cputask_arg_layer<R(As...)>* c_ptr2(::symphony::task<R(As...)>& t);
};

template<typename Task>
symphony::internal::by_data_dep_t<Task&&>
bind_as_data_dependency(Task&& t ) {
  return symphony::internal::by_data_dep_t<Task&&>(std::forward<Task>(t));
}

template<typename BlockingFunction, typename CancelFunction>
void blocking(BlockingFunction&& bf, CancelFunction&& cf);

template<typename ReturnType, typename ...Args>
SYMPHONY_CONSTEXPR_CONST typename task<ReturnType(Args...)>::size_type
task<ReturnType(Args...)>::arity;

template<typename Task>
symphony::internal::by_value_t<Task&&> bind_by_value(Task&& t){
  return symphony::internal::by_value_t<Task&&>(std::forward<Task>(t));
}

inline void abort_on_cancel() {
  auto t = internal::current_task();
  SYMPHONY_API_THROW(t, "Error: you must call abort_on_cancel from within a task.");
  t->abort_on_cancel();
}

void abort_task();

namespace internal {

inline
::symphony::internal::task* c_ptr(::symphony::task<>* t) {
  return t->get_raw_ptr();
}

template<typename R, typename... As>
::symphony::internal::cputask_arg_layer<R(As...)>* c_ptr2(::symphony::task<R(As...)>& t) {
  return t.get_typed_raw_ptr();
}

};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
