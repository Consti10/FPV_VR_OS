// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/taskptr.hh>

#include <symphony/internal/legacy/attr.hh>

namespace symphony {
namespace internal{

template<typename UserCode>
class cpu_kernel {

public:

  using user_code_traits = function_traits<UserCode>;
  using user_code_type_in_task = typename user_code_traits::type_in_task;

  using collapsed_task_type_info = task_type_info<UserCode, true>;
  using non_collapsed_task_type_info = task_type_info<UserCode, false>;

  using collapsed_signature = typename collapsed_task_type_info::final_signature;
  using non_collapsed_signature = typename non_collapsed_task_type_info::final_signature;

  using collapsed_raw_task_type = cputask_arg_layer<collapsed_signature>;
  using non_collapsed_raw_task_type = cputask_arg_layer<non_collapsed_signature>;

  using anonymous_task_type_info = non_collapsed_task_type_info;
  using anonymous_raw_task_type = non_collapsed_raw_task_type;

  using collapsed_task_type = ::symphony::task_ptr<collapsed_signature>;
  using non_collapsed_task_type = ::symphony::task_ptr<non_collapsed_signature>;

  using size_type = typename user_code_traits::size_type;
  using return_type = typename user_code_traits::return_type;
  using args_tuple = typename user_code_traits::args_tuple;

  static SYMPHONY_CONSTEXPR_CONST size_type arity =user_code_traits::arity::value;

  explicit cpu_kernel(UserCode&& user_code) :
    _user_code(std::forward<UserCode>(user_code)),
    _attrs(legacy::create_task_attrs(::symphony::internal::legacy::attr::cpu)) {

   }

  explicit cpu_kernel(UserCode const& user_code) :
    _user_code(user_code),
    _attrs(legacy::create_task_attrs(::symphony::internal::legacy::attr::cpu)) {

   }

  cpu_kernel(cpu_kernel const& other) :
    _user_code(other._user_code),
    _attrs(other._attrs) {

  }

  cpu_kernel(cpu_kernel && other) :
    _user_code(std::move(other._user_code)),
    _attrs(std::move(other._attrs)) {

  }

  cpu_kernel& operator=(cpu_kernel const& other) {
    static_assert(std::is_copy_assignable<UserCode>::value,
                  "Only bodies created using copy-assignable functors or function pointers are copy assignable.");

    if (&other == this)
      return *this;

    _user_code = other._user_code;
    _attrs = other._attrs;
    return *this;
  }

  cpu_kernel& operator=(cpu_kernel&& other) {
    if(&other == this)
      return *this;
    _user_code = std::move(other._user_code);
    _attrs = other._attrs;
    other._attrs = legacy::create_task_attrs(::symphony::internal::legacy::attr::cpu);
    return *this;
  }

  template<typename Attribute>
  void set_attr(Attribute const& attr) {
    _attrs = ::symphony::internal::legacy::add_attr(_attrs, attr);
  }

  template<typename Attribute>
  bool has_attr(Attribute const& attr) const {
    return ::symphony::internal::legacy::has_attr(_attrs, attr);
  }

  user_code_type_in_task& get_fn() {
    return _user_code;
  }

  ::symphony::internal::legacy::task_attrs get_attrs() {
    return _attrs;
  }

private:
  user_code_type_in_task _user_code;
  ::symphony::internal::legacy::task_attrs _attrs;
};

};
};
