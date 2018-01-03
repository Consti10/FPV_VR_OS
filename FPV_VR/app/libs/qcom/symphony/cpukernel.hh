// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/legacy/attr.hh>
#include <symphony/internal/task/cpukernel.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {
namespace internal {

template<typename X, typename Y> struct task_factory_dispatch;
template<typename X, typename Y, typename Z> struct task_factory;
struct cpu_kernel_caller;

};

template<typename Fn>
class cpu_kernel : protected ::symphony::internal::cpu_kernel<Fn> {

  using parent = ::symphony::internal::cpu_kernel<Fn>;

  template<typename X, typename Y>
  friend struct ::symphony::internal::task_factory_dispatch;

  template<typename X, typename Y, typename Z>
  friend struct ::symphony::internal::task_factory;

  friend struct ::symphony::internal::cpu_kernel_caller;

  static_assert(std::is_copy_constructible<Fn>::value,
                "CPU kernels must be copy constructible.");

  static_assert(std::is_move_constructible<Fn>::value,
                "CPU kernels must be move constructible.");

public:

  using size_type = typename parent::size_type;
  using return_type = typename parent::return_type;
  using args_tuple = typename parent::args_tuple;

  using collapsed_task_type = typename parent::collapsed_task_type;
  using non_collapsed_task_type = typename parent::non_collapsed_task_type;

  static SYMPHONY_CONSTEXPR_CONST size_type arity = parent::arity;

  explicit cpu_kernel(Fn const& fn) :
    parent(fn) {
  }

  explicit cpu_kernel(Fn&& fn) :
    parent(std::forward<Fn>(fn)) {
  }

  cpu_kernel(cpu_kernel const& other) :
    parent(other) {
  }

  cpu_kernel(cpu_kernel && other) :
    parent(std::move(other)) {
  }

  cpu_kernel& operator=(cpu_kernel const& other) {
    return static_cast<cpu_kernel&>(parent::operator=(other));
  }

  cpu_kernel& operator=(cpu_kernel&& other) {
    return static_cast<cpu_kernel&>(parent::operator=(std::move(other)));
  }

  cpu_kernel& set_blocking() {
    parent::set_attr(::symphony::internal::legacy::attr::blocking);
    return *this;
  }

  bool is_blocking() const {
    return parent::has_attr(::symphony::internal::legacy::attr::blocking);
  }

  cpu_kernel& set_big() {
    parent::set_attr(::symphony::internal::legacy::attr::big);
    return *this;
  }

  bool is_big() const {
    return parent::has_attr(::symphony::internal::legacy::attr::big);
  }

  cpu_kernel& set_little() {
    parent::set_attr(::symphony::internal::legacy::attr::little);
    return *this;
  }

  bool is_little() const {
    return parent::has_attr(::symphony::internal::legacy::attr::little);
  }

  ~cpu_kernel() { }
};

template<typename Fn>
SYMPHONY_CONSTEXPR_CONST typename cpu_kernel<Fn>::size_type cpu_kernel<Fn>::arity;

template <typename FReturnType, typename... FArgs>
class cpu_kernel<FReturnType(FArgs...)> : protected ::symphony::internal::cpu_kernel<FReturnType(*)(FArgs...)> {

  using parent = ::symphony::internal::cpu_kernel<FReturnType(*)(FArgs...)>;

  template<typename X, typename Y>
  friend struct ::symphony::internal::task_factory_dispatch;

  template<typename X, typename Y, typename Z>
  friend struct ::symphony::internal::task_factory;

  friend struct ::symphony::internal::cpu_kernel_caller;

public:

  using size_type = typename parent::size_type;
  using return_type = typename parent::return_type;
  using args_tuple = typename parent::args_tuple;

  using collapsed_task_type = typename parent::collapsed_task_type;
  using non_collapsed_task_type = typename parent::non_collapsed_task_type;

  static SYMPHONY_CONSTEXPR_CONST size_type arity = parent::arity;

  explicit cpu_kernel(FReturnType(*fn)(FArgs...)) :
  parent(std::forward<FReturnType(*)(FArgs...)>(fn)) {
   }

  cpu_kernel(cpu_kernel const& other) :
    parent(other) {
  }

  cpu_kernel(cpu_kernel&& other) :
    parent(other) {
  }

  cpu_kernel& operator=(cpu_kernel const& other) {
    return static_cast<cpu_kernel&>(parent::operator=(other));
  }

  cpu_kernel& operator=(cpu_kernel&& other) {
    return static_cast<cpu_kernel&>(parent::operator=(std::move(other)));
  }

  cpu_kernel& set_blocking() {
    parent::set_attr(::symphony::internal::legacy::attr::blocking);
    return *this;
  }

  bool is_blocking() const {
    return parent::has_attr(::symphony::internal::legacy::attr::blocking);
  }

  cpu_kernel& set_big() {
    parent::set_attr(::symphony::internal::legacy::attr::big);
    return *this;
  }

  bool is_big() const {
    return parent::has_attr(::symphony::internal::legacy::attr::big);
  }

  cpu_kernel& set_little() {
    parent::set_attr(::symphony::internal::legacy::attr::little);
    return *this;
  }

  bool is_little() const {
    return parent::has_attr(::symphony::internal::legacy::attr::little);
  }

  ~cpu_kernel() { }
};

template <typename FReturnType, typename... FArgs>
SYMPHONY_CONSTEXPR_CONST typename symphony::cpu_kernel<FReturnType(FArgs...)>::size_type
  symphony::cpu_kernel<FReturnType(FArgs...)>::arity;

template <typename FReturnType, typename... FArgs>
symphony::cpu_kernel<FReturnType(FArgs...)> create_cpu_kernel(FReturnType(*fn)(FArgs...)) {
  return symphony::cpu_kernel<FReturnType(FArgs...)>(fn);
}

template <typename Fn>
symphony::cpu_kernel<typename std::remove_reference<Fn>::type> create_cpu_kernel(Fn&& fn) {
  using no_ref = typename std::remove_reference<Fn>::type;
  return symphony::cpu_kernel<no_ref>(std::forward<Fn>(fn));
}

};

SYMPHONY_GCC_IGNORE_END("-Weffc++");
