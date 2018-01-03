// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/task/functiontraits.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/demangler.hh>
#include <symphony/internal/util/memorder.hh>
#include <symphony/internal/util/templatemagic.hh>

namespace symphony {

template<typename ...Stuff> class task_ptr;
template<typename ...Stuff> class task;

namespace pattern {
  template<typename T1, typename T2>       class pfor;
  template<typename Reduce, typename Join> class preducer;
  template<typename Fn>                    class ptransformer;
  template<typename BinaryFn>              class pscan;
  template<typename Compare>               class psorter;
  template<typename IsBaseFn, typename BaseFn, typename SplitFn, typename MergeFn>
  class pdivide_and_conquerer;
};

namespace internal {

::symphony::internal::task* c_ptr(::symphony::task_ptr<>& t);
::symphony::internal::task* c_ptr(::symphony::task_ptr<> const& t);
::symphony::internal::task* c_ptr(::symphony::task<>*);

template <typename ReturnType>
class cputask_return_layer;

namespace testing {

class ArgTrackerTest;
class TaskArgTypeInfoTest;

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename UserCode, bool> class user_code_container_base;

template<typename UserCode>
class user_code_container_base<UserCode, true> {
protected:

  using user_code_traits = function_traits<UserCode>;
  using user_code_type_in_task = typename user_code_traits::type_in_task;

#ifndef _MSC_VER
private:

  union {
    user_code_type_in_task _user_code;
  };

protected:

  template <typename Code = UserCode>
  explicit user_code_container_base(Code&& code) :
    _user_code(std::forward<Code>(code)) {
  }

  user_code_type_in_task& get_user_code() {
    return _user_code;
  }

public:

  void destroy_user_code() {
    _user_code.~user_code_type_in_task();
  }

#else

private:
  user_code_type_in_task* _user_code;

protected:

  template <typename Code = UserCode>
  explicit user_code_container_base(Code&& user_code) :
    _user_code(new user_code_type_in_task(std::forward<Code>(user_code))) {
  }

  user_code_type_in_task& get_user_code() {
    return *_user_code;
  }

public:

  void destroy_user_code() {
    delete(_user_code);
    _user_code = nullptr;
  }

#endif

  ~user_code_container_base(){}

  SYMPHONY_DELETE_METHOD(user_code_container_base(user_code_container_base const&));
  SYMPHONY_DELETE_METHOD(user_code_container_base(user_code_container_base&&));
  SYMPHONY_DELETE_METHOD(user_code_container_base& operator=(user_code_container_base const&));
  SYMPHONY_DELETE_METHOD(user_code_container_base& operator=(user_code_container_base&&));
};

template<typename UserCode>
class user_code_container_base<UserCode, false> {

protected:

  using user_code_traits = function_traits<UserCode>;
  using user_code_type_in_task = typename user_code_traits::type_in_task;

  user_code_type_in_task& get_user_code() {
    return _user_code;
  }

private:
  user_code_type_in_task _user_code;

public:

  template <typename Code=UserCode>
  explicit user_code_container_base(Code&& user_code) :
    _user_code(user_code) {
  }

  ~user_code_container_base(){}

public:

  void destroy_user_code() {}

  SYMPHONY_DELETE_METHOD(user_code_container_base(user_code_container_base const&));
  SYMPHONY_DELETE_METHOD(user_code_container_base(user_code_container_base&&));
  SYMPHONY_DELETE_METHOD(user_code_container_base& operator=(user_code_container_base const&));
  SYMPHONY_DELETE_METHOD(user_code_container_base& operator=(user_code_container_base&&));
};

template<typename UserCode>
class user_code_container : public user_code_container_base<UserCode,
                                                            function_traits<UserCode>::has_destructor::value> {

  using parent = user_code_container_base<UserCode,
                                          function_traits<UserCode>::has_destructor::value>;
public:

  using user_code_type = UserCode;
  using return_type = typename function_traits<UserCode>::return_type;

  template <typename Code=UserCode>
  explicit user_code_container(Code&& body) :
    parent(std::forward<Code>(body)) {
  }

  ~user_code_container() {}

  template <typename ...UserCodeArgs>
  return_type operator()(UserCodeArgs&& ...args) {
    return parent::get_user_code()(std::forward<UserCodeArgs>(args)...);
  }

  uintptr_t get_source() const {
#ifdef SYMPHONY_HAVE_RTTI
    return reinterpret_cast<uintptr_t>(&typeid(const_cast<user_code_container*>(this)->get_user_code()));
#else
    return 0;
#endif
  }

  std::string to_string() {
#ifdef SYMPHONY_HAVE_RTTI
    return demangler::demangle(typeid(parent::get_user_code()));
#else
    return std::string("Enable RTTI for demangling class name.");
#endif
  }

  SYMPHONY_DELETE_METHOD(user_code_container(user_code_container const&));
  SYMPHONY_DELETE_METHOD(user_code_container(user_code_container&&));
  SYMPHONY_DELETE_METHOD(user_code_container& operator=(user_code_container const&));
  SYMPHONY_DELETE_METHOD(user_code_container& operator=(user_code_container&&));
};

class set_arg_tracker {

public:

  using size_type = std::uint32_t;

private:

  using mask_type = std::uint32_t;
  using min_arity = std::integral_constant<mask_type, 1>;

  std::atomic<mask_type> _args;

  friend class testing::ArgTrackerTest;

public:

  using do_not_track = bool;

  using max_arity = std::integral_constant<mask_type, (sizeof(mask_type) * 8) - 1>;

  explicit set_arg_tracker(size_type arity) :
    _args( (1 << arity) - 1 ) {
    SYMPHONY_INTERNAL_ASSERT(arity >= min_arity::value,
                         "Arity should be greated than %d.",
                         static_cast<int>(min_arity::value));

    SYMPHONY_INTERNAL_ASSERT(arity <= max_arity::value,
                         "Arity is larger than %d: %d.",
                         static_cast<int>(max_arity::value),
                         static_cast<int>(arity));
   }

  explicit set_arg_tracker(do_not_track) :
    _args(0) {
   }

  std::tuple<bool, bool> set(size_type pos) {

    SYMPHONY_INTERNAL_ASSERT(pos <= max_arity::value, "Out of range pos: %d", pos);

    const mask_type mask = ~(1 << (pos));

    mask_type desired;
    mask_type expected = _args.load(symphony::mem_order_relaxed);

    do {
      desired = expected & mask;

      if (desired == expected) {
        return std::make_tuple(false, desired == 0);
      }

    } while (!std::atomic_compare_exchange_weak_explicit(&_args,
                                                         &expected,
                                                         desired,
                                                         symphony::mem_order_acq_rel,
                                                         symphony::mem_order_acquire));

    return std::make_tuple(true, desired == 0);
  }

  std::tuple<bool, size_type> set_all(size_type arity) {
    mask_type expected = (1 << arity) - 1;
    mask_type desired = 0;

    if (std::atomic_compare_exchange_strong_explicit(&_args,
                                                     &expected,
                                                     desired,
                                                     symphony::mem_order_acq_rel,
                                                     symphony::mem_order_acquire)) {
      return std::make_tuple(true, 0);
    }

    return std::make_tuple(false, get_first_set_arg(expected));
  }

  bool are_all_set() {
    return _args.load(symphony::mem_order_acquire) == 0;
  }

private:

  static size_type get_first_set_arg(mask_type mask);

};

template<typename T> struct arg_container;

template<typename T>
struct arg_container {

  using type = T;

  static_assert(std::is_default_constructible<T>::value,
                "Task arguments types must be default_constructible.");

  static_assert(std::is_copy_assignable<T>::value,
                "Task argument must be copy_assignable.");

  static_assert(std::is_copy_constructible<T>::value,
                "Task argument must be copy_constructible");

  arg_container() :
    _arg() {
  }

  explicit arg_container(type const& value) :
    _arg(value) {
  }

  explicit arg_container(type&& value) :
    _arg(std::move(value)) {
  }

  arg_container& operator=(type const& value) {
    _arg = value;
    return *this;
  }

  arg_container& operator=(type&& value) {
    _arg = std::move(value);
    return *this;
  }

   operator type&() {
    return _arg;
  }

  type& get_larg() {
    return _arg;
  }

  void destroy() {
    _arg.~type();
  }

  ~arg_container(){}

  SYMPHONY_DELETE_METHOD(arg_container(arg_container const& other_arg));
  SYMPHONY_DELETE_METHOD(arg_container(arg_container&& other_arg));
  SYMPHONY_DELETE_METHOD(arg_container& operator=(arg_container const&));
  SYMPHONY_DELETE_METHOD(arg_container& operator=(arg_container&&));

private:

  union {
    type _arg;
  };
};

template<typename T>
struct arg_container<T&&> {

  using type = T;

  static_assert(std::is_default_constructible<T>::value,
                "Task arguments types must be default_constructible.");

  static_assert(std::is_move_constructible<T>::value,
                "Task argument must be move_constructible.");

  static_assert(std::is_move_assignable<T>::value,
                "Task argument must be move_assignable.");

  arg_container() :
    _arg() {
  }

  explicit arg_container(type&& value) :
    _arg(std::move(value)) {
  }

  arg_container& operator=(type&& value) {
    _arg = std::move(value);
    return *this;
  }

   operator type&&() {
    return std::move(_arg);
  }

  type& get_larg() {
    return _arg;
  }

  void destroy() {
    _arg.~type();
  }

  ~arg_container(){}

private:

  union {
    type _arg;
  };

  SYMPHONY_DELETE_METHOD(arg_container(type const&));
  SYMPHONY_DELETE_METHOD(arg_container(arg_container const& other_arg));
  SYMPHONY_DELETE_METHOD(arg_container(arg_container&& other_arg));
  SYMPHONY_DELETE_METHOD(arg_container& operator=(arg_container const&));
  SYMPHONY_DELETE_METHOD(arg_container& operator=(arg_container&&));
};

template<typename T>
struct retval_container {

  using orig_type = T;
  using type = typename std::remove_cv<T>::type;

  static_assert(!std::is_reference<T>::value,
                "Task return types cannot be references.");

  static_assert(!std::is_volatile<T>::value,
                "Task return types cannot be volatile.");

  static_assert(std::is_move_constructible<type>::value,
                "Task return types must be move_constructible.");

  static_assert(std::is_move_assignable<type>::value,
                "Task return types must be move_assignable.");

  retval_container() :
    _retval() {
  }

  explicit retval_container(type const& value) :
    _retval(value) {
  }

  explicit retval_container(type&& value) :
    _retval(std::move(value)) {
  }

  retval_container& operator=(type const& value) {
    _retval = value;
    return *this;
  }

  retval_container& operator=(type&& value) {
    _retval = std::move(value);
    return *this;
  }

  ~retval_container(){}

  SYMPHONY_DELETE_METHOD(retval_container(retval_container const& other_arg));
  SYMPHONY_DELETE_METHOD(retval_container(retval_container&& other_arg));
  SYMPHONY_DELETE_METHOD(retval_container& operator=(retval_container const&));
  SYMPHONY_DELETE_METHOD(retval_container& operator=(retval_container&&));

private:

  type _retval;

  template <typename ReturnType>
  friend class cputask_return_layer;

};

template< class T >
struct task_arg_type_info {

  using orig_type = T;

  using no_ref_type = typename std::remove_reference<orig_type>::type;

  using no_ref_no_cv_type = typename std::remove_cv<no_ref_type>::type;

  using decayed_type = typename std::decay<orig_type>::type;

  using is_lvalue_ref = typename std::is_lvalue_reference<orig_type>::type;

  using is_rvalue_ref = typename std::is_rvalue_reference<orig_type>::type;

  using storage_type = typename std::conditional<is_rvalue_ref::value,
                                                 arg_container<decayed_type&&>,
                                                 arg_container<decayed_type>>::type;

  static_assert(is_lvalue_ref::value == false,
                "This version of Symphony does not support lvalue references as task arguments. "
                "Please check the Symphony manual.");

  static_assert(std::is_default_constructible<decayed_type>::value,
                "Task arguments types must be default_constructible.");

  using valid_rvalue_ref = typename std::conditional<is_rvalue_ref::value,
                                                     typename std::conditional<std::is_move_constructible<no_ref_type>::value &&
                                                                               std::is_move_assignable<no_ref_type>::value,
                                                                               std::true_type,
                                                                               std::false_type>::type,
                                                     std::true_type
                                                     >::type;

  static_assert(valid_rvalue_ref::value,
                "Rvalue reference arguments must be move constructible and move assignable.");

  using valid_lvalue = typename std::conditional<is_rvalue_ref::value == false,
                                                 typename std::conditional<std::is_copy_constructible<decayed_type>::value &&
                                                                           std::is_copy_assignable<decayed_type>::value,
                                                                           std::true_type,
                                                                           std::false_type>::type,
                                                 std::true_type
                                                 >::type;

  static_assert(valid_lvalue::value,
                "lvalue arguments must be copy constructible and copy assignable.");

  using is_valid = typename std::conditional<(is_lvalue_ref::value == false &&
                                              valid_rvalue_ref::value &&
                                              valid_lvalue::value),
                                             std::true_type,
                                             std::false_type>::type;

  static_assert(!std::is_volatile<orig_type>::value,
                "This version of symphony does not support volatile task arguments.");
};

template<typename ArgTuple, std::size_t pos>
struct check_arg {
  using orig_type = typename std::tuple_element<pos, ArgTuple>::type;
  using type_info = task_arg_type_info<orig_type>;
  using is_valid = typename std::conditional<type_info::is_valid::value,
                                             typename check_arg<ArgTuple, pos - 1>::is_valid,
                                             std::false_type>::type;
};

template<typename ArgTuple>
struct check_arg<ArgTuple, 0> {
  using orig_type = typename std::tuple_element<0, ArgTuple>::type;
  using type_info = task_arg_type_info<orig_type>;
  using is_valid = typename type_info::is_valid;
};

template <typename ArgsTuple, std::size_t arity>
struct check_body_args {
   using all_valid = typename check_arg<ArgsTuple, arity - 1>::is_valid;
};

template <typename ArgsTuple>
struct check_body_args<ArgsTuple, 0> {
  using all_valid = std::true_type;
};

template <typename T>
struct is_symphony_task20_ptr {
  using type = T;
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = false;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = false;
};

template <typename T>
struct is_symphony_task20_ptr<::symphony::task_ptr<T>> {
  using type = T;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = false;
};

template <>
struct is_symphony_task20_ptr<::symphony::task_ptr<>> {
  using type = void;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = false;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = false;
};

template <typename R, typename... Args>
struct is_symphony_task20_ptr<::symphony::task_ptr<R(Args...)>> {
  using type = R;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = true;
};

template <typename T>
struct is_symphony_task20_ptr<::symphony::task<T>*> {
  using type = T;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = false;
};

template <>
struct is_symphony_task20_ptr<::symphony::task<>*> {
  using type = void;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = false;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = false;
};

template <typename R, typename... Args>
struct is_symphony_task20_ptr<::symphony::task<R(Args...)>*> {
  using type = R;
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_return_value = true;
  static SYMPHONY_CONSTEXPR_CONST bool has_arguments = true;
};

template<typename F>
struct is_pattern {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
};

template<typename UnaryFn>
struct is_pattern<::symphony::pattern::pfor<UnaryFn, void>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename Fn>
struct is_pattern<::symphony::pattern::ptransformer<Fn>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename BinaryFn>
struct is_pattern<::symphony::pattern::pscan<BinaryFn>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename Compare>
struct is_pattern<::symphony::pattern::psorter<Compare>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename IsBaseFn, typename BaseFn, typename SplitFn, typename MergeFn>
struct is_pattern<::symphony::pattern::pdivide_and_conquerer<IsBaseFn, BaseFn, SplitFn, MergeFn>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename F, typename Enable = void>
struct is_group_launchable {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
};

template<typename Reduce, typename Join>
struct is_group_launchable<::symphony::pattern::preducer<Reduce, Join>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
};

template<typename IsBaseFn, typename BaseFn, typename SplitFn, typename MergeFn>
struct is_group_launchable<::symphony::pattern::pdivide_and_conquerer<IsBaseFn, BaseFn, SplitFn, MergeFn>,
                            typename std::enable_if<!std::is_same
                              <typename internal::function_traits<BaseFn>::return_type,void>::value>::type> {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
};

template<typename NewReturnType, typename ReturnType, typename... Args>
struct substitute_return_type;

template<typename NewReturnType, typename ReturnType, typename... Args>
struct substitute_return_type<NewReturnType, ReturnType(Args...)> {

  typedef NewReturnType type(Args...);
};

template<typename UserCode, bool CollapseRequested>
struct task_type_info {

  using user_code = UserCode;

  using user_code_traits = function_traits<UserCode>;
  using container = user_code_container<UserCode>;

  using collapse_requested =
      typename std::integral_constant<bool, CollapseRequested>;
  using collapse_actual =
      typename std::integral_constant<bool,
      CollapseRequested &&
      is_symphony_task20_ptr<typename user_code_traits::return_type>::value>;

  using collapsed_return_type =
      typename std::conditional<collapse_actual::value,
      typename is_symphony_task20_ptr<typename user_code_traits::return_type>::type,
      typename user_code_traits::return_type>::type;

  using collapsed_signature = typename substitute_return_type<collapsed_return_type,
                                                              typename user_code_traits::signature>::type;

  using final_signature = typename std::conditional<collapse_actual::value,
                                                    collapsed_signature,
                                                    typename user_code_traits::signature>::type;

  using size_type = std::size_t;

  static_assert(std::is_reference<typename user_code_traits::return_type>::value == false,
                "This version of Symphony does not support references as the return type of tasks."
                "Please check the Symphony manual.");

  static_assert(user_code_traits::arity::value < set_arg_tracker::max_arity::value,
                "Task body has too many arguments. Please refer to the manual.");

  using args_tuple = typename user_code_traits::args_tuple;
  using args_arity = typename user_code_traits::arity;
  using args_all_valid = typename check_body_args<args_tuple, args_arity::value>::all_valid;

  static_assert(args_all_valid::value, "Task body has illegal argument types.");
};

template<typename TaskType>
struct by_value_t {

  explicit by_value_t(TaskType&& t) : _t(std::forward<TaskType>(t)) {};

  TaskType&& _t;
};

template <typename T >
struct is_by_value_helper_t {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
  using type = T;
};

template <typename T>
struct is_by_value_helper_t<by_value_t<T>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  using type                  = T;
};

template <typename T >
struct is_by_value_t {

private:
  using no_ref = typename std::remove_reference<T>::type;

public:
  using helper                = typename ::symphony::internal::is_by_value_helper_t<no_ref>;
  static SYMPHONY_CONSTEXPR_CONST bool value = helper::value;
  using type                  = typename std::conditional<value,
                                                          typename helper::type,
                                                          T>::type;
};

template<typename TaskType>
struct by_data_dep_t {

  explicit by_data_dep_t(TaskType&& t) : _t(std::forward<TaskType>(t)) {};

  TaskType&& _t;
};

template <typename T >
struct is_by_data_dep_helper_t {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
  using type = T;
};

template <typename T>
struct is_by_data_dep_helper_t<by_data_dep_t<T>> {
  static SYMPHONY_CONSTEXPR_CONST bool value = true;
  using type                  = T;
};

template <typename T >
struct is_by_data_dep_t {

private:
  using no_ref = typename std::remove_reference<T>::type;

public:
  using helper                = typename ::symphony::internal::is_by_data_dep_helper_t<no_ref>;
  static SYMPHONY_CONSTEXPR_CONST bool value = helper::value;
  using type                  = typename std::conditional<value,
                                                          typename helper::type,
                                                          T>::type;
};

template<typename UserProvided, typename Expected>
class can_bind_as_value {

private:
  static void test(Expected);

  template <typename U>
  static auto check(int) -> decltype(test(std::declval<U>()), std::true_type());

  template <typename>
  static std::false_type check(...);

  using enable_if_type = decltype(check<UserProvided>(0)) ;

public:
  using type = typename enable_if_type::type;
  static SYMPHONY_CONSTEXPR_CONST bool value = type::value;

};

template<typename PredType, typename Expected, typename Enabled = void>
struct can_bind_as_data_dep {
  static SYMPHONY_CONSTEXPR_CONST bool value = false;
};

template<typename PredType, typename Expected>
struct can_bind_as_data_dep<
  PredType,
  Expected,
  typename std::enable_if<
    is_symphony_task20_ptr<
      typename std::remove_cv<
        typename std::remove_reference<PredType>::type
      >::type
    >::has_return_value
  >::type> {

private:
  using pred_type_nocvref =
    typename std::remove_cv<typename std::remove_reference<PredType>::type>::type;
  using return_type       = typename ::symphony::internal::is_symphony_task20_ptr<pred_type_nocvref>::type;

public:
  static SYMPHONY_CONSTEXPR_CONST bool value =
    (std::is_same<void, return_type>::value == false) &&
    can_bind_as_value<return_type, Expected>::value;

};

template<typename ArgType>
static ::symphony::internal::task* get_cptr(ArgType* arg) {
  using decayed_argtype = typename std::decay<ArgType*>::type;
  static_assert(is_symphony_task20_ptr<decayed_argtype>::value, "");

  return ::symphony::internal::c_ptr(static_cast<::symphony::task<>*>(arg));
}

template<typename ArgType>
static ::symphony::internal::task* get_cptr(ArgType&& arg) {
  using decayed_argtype = typename std::decay<ArgType>::type;
  static_assert(is_symphony_task20_ptr<decayed_argtype>::value, "");

  return ::symphony::internal::c_ptr(arg);
}

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
