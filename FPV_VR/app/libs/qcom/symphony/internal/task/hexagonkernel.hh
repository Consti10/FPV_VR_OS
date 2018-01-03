// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if defined SYMPHONY_HAVE_QTI_HEXAGON

#include <symphony/taskptr.hh>

#include <symphony/internal/legacy/attr.hh>

namespace symphony {
namespace internal{

template<typename ...Args>
class hexagon_kernel {

public:

  using dsp_code_type = int(*)(Args...);
  using user_code_traits = function_traits<dsp_code_type>;

  using collapsed_task_type_info = task_type_info<dsp_code_type, true>;
  using non_collapsed_task_type_info = task_type_info<dsp_code_type, false>;

  using collapsed_signature = typename collapsed_task_type_info::final_signature;
  using non_collapsed_signature = typename non_collapsed_task_type_info::final_signature;

  using collapsed_raw_task_type = hexagontask_arg_layer<collapsed_signature>;
  using non_collapsed_raw_task_type = hexagontask_arg_layer<non_collapsed_signature>;

  using anonymous_task_type_info = non_collapsed_task_type_info;
  using anonymous_raw_task_type = non_collapsed_raw_task_type;

  using collapsed_task_type = ::symphony::task_ptr<collapsed_signature>;
  using non_collapsed_task_type = ::symphony::task_ptr<non_collapsed_signature>;

  using size_type = typename user_code_traits::size_type;
  using return_type = typename user_code_traits::return_type;
  using args_tuple = typename user_code_traits::args_tuple;

  static SYMPHONY_CONSTEXPR_CONST size_type arity = user_code_traits::arity::value;

  explicit hexagon_kernel(dsp_code_type& fn) :
    _fn(std::forward<dsp_code_type>(fn))
    , _attrs(legacy::create_task_attrs(legacy::attr::hexagon)) {
   }

  explicit hexagon_kernel(dsp_code_type const& fn) :
    _fn(fn)
    , _attrs(legacy::create_task_attrs(legacy::attr::hexagon)) {
   }

  hexagon_kernel(hexagon_kernel const& other) :
    _fn(other._fn),
    _attrs(other._attrs) {

  }

  hexagon_kernel(hexagon_kernel && other) :
    _fn(std::move(other._fn)),
    _attrs(std::move(other._attrs)) {

  }

  hexagon_kernel& operator=(hexagon_kernel const& other) {

    if (&other == this)
      return *this;

    _fn = other._fn;
    _attrs = other._attrs;
    return *this;
  }

  hexagon_kernel& operator=(hexagon_kernel&& other) {
    if(&other == this)
      return *this;
    _fn = std::move(other._fn);
    _attrs = other._attrs;
    other._attrs = legacy::create_task_attrs(legacy::attr::hexagon);
    return *this;
  }

  template<typename Attribute>
  void set_attr(Attribute const& attr) {
    _attrs = legacy::add_attr(_attrs, attr);
  }

  template<typename Attribute>
  bool has_attr(Attribute const& attr) const {
    return legacy::has_attr(_attrs, attr);
  }

  dsp_code_type& get_fn() {
    return _fn;
  }

  legacy::task_attrs get_attrs() {
    return _attrs;
  }

private:
  dsp_code_type _fn;
  legacy::task_attrs _attrs;
};

};
};
#endif
