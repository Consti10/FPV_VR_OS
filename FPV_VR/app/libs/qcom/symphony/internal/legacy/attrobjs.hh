// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstdint>
#include <tuple>
#include <utility>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/task/functiontraits.hh>

namespace symphony  {

template <typename... Kargs>
class gpu_kernel;

namespace internal{
namespace legacy{

class task_attrs;

template <typename... TS> class body_with_attrs;

#ifdef SYMPHONY_HAVE_GPU
template <typename... TS> class body_with_attrs_gpu;

template <typename Body, typename KernelPtr, typename... Kargs>
SYMPHONY_CONSTEXPR
body_with_attrs_gpu<Body, KernelPtr, Kargs...>  with_attrs_gpu_internal(
                                                task_attrs const& attrs,
                                                Body&& body,
                                                KernelPtr kernel,
                                                Kargs... args);

SYMPHONY_CLANG_IGNORE_BEGIN("-Wpredefined-identifier-outside-function");
SYMPHONY_CLANG_IGNORE_BEGIN("-Wunneeded-internal-declaration");

static auto default_cpu_kernel = []() {
  SYMPHONY_UNIMPLEMENTED("cpu version of task not provided");
};

typedef decltype(default_cpu_kernel) CpuKernelType;

SYMPHONY_CLANG_IGNORE_END("-Wunneeded-internal-declaration");
SYMPHONY_CLANG_IGNORE_END("-Wpredefined-identifier-outside-function");

template <typename KernelPtr, typename... Kargs>
SYMPHONY_CONSTEXPR body_with_attrs_gpu<CpuKernelType, KernelPtr, Kargs...>
with_attrs_gpu(task_attrs const& attrs, KernelPtr kernel, Kargs... args);

#endif

template <typename Body>
SYMPHONY_CONSTEXPR body_with_attrs<Body>
with_attrs_cpu(task_attrs const& attrs, Body&& body);

template <typename Body>
SYMPHONY_CONSTEXPR body_with_attrs<Body>
with_attrs_cpu(task_attrs const& attrs, Body&& body, std::nullptr_t);

template <typename Body, typename Cancel_Handler>
SYMPHONY_CONSTEXPR body_with_attrs<Body, Cancel_Handler>
with_attrs_cpu(task_attrs const& attrs,
               Body&& body,
               Cancel_Handler&& handler);

template<typename Body, typename T, typename... Kargs>
struct with_attrs_dispatcher;

SYMPHONY_CONSTEXPR task_attrs create_task_attrs();

SYMPHONY_CONSTEXPR bool
operator== (symphony::internal::legacy::task_attrs const& a, symphony::internal::legacy::task_attrs const& b);
SYMPHONY_CONSTEXPR bool
operator!= (symphony::internal::legacy::task_attrs const& a, symphony::internal::legacy::task_attrs const& b);

class task_attrs {
public:

  SYMPHONY_CONSTEXPR task_attrs(task_attrs const& other):
    _mask(other._mask)
  {}

  task_attrs& operator=(task_attrs const& other) {
    _mask = other._mask;
    return *this;
  }

#ifndef _MSC_VER
  friend constexpr bool ::symphony::internal::legacy::operator== (task_attrs const& a,
                                                              task_attrs const& b);
  friend constexpr bool ::symphony::internal::legacy::operator!= (task_attrs const& a,
                                                              task_attrs const& b);
#endif

#ifndef _MSC_VER
private:
  constexpr
#endif
  task_attrs():_mask(0){}

  SYMPHONY_CONSTEXPR explicit task_attrs(std::int32_t mask):_mask(mask){}

  std::int32_t _mask;

#ifndef _MSC_VER
  friend constexpr task_attrs create_task_attrs();

  template<typename Attribute, typename...Attributes>
  friend constexpr task_attrs create_task_attrs(Attribute const&,
                                                Attributes const& ...);

  template<typename Attribute>
  friend constexpr bool has_attr(task_attrs const& attrs,
                                 Attribute const& attr);

  template<typename Attribute>
  friend constexpr task_attrs remove_attr(task_attrs const& attrs,
                                          Attribute const& attr);

  template<typename Attribute>
  friend const task_attrs add_attr(task_attrs const& attrs,
                                   Attribute const& attr);
#endif
};

template <typename Body>
class body_with_attrs<Body>
{
public:

  typedef internal::function_traits<Body> body_traits;

  typedef typename body_traits::return_type return_type;

  SYMPHONY_CONSTEXPR task_attrs const& get_attrs() const { return _attrs;}

  SYMPHONY_CONSTEXPR Body& get_body() const { return _body; }

  template<typename ...Args>
  SYMPHONY_CONSTEXPR return_type operator()(Args... args) const {
    return (get_body())(args...);
  }

#ifndef _MSC_VER
private:
  constexpr
#endif
  body_with_attrs(task_attrs const& attrs, Body&& body):
    _attrs(attrs),
    _body(body){
  }

  task_attrs const& _attrs;
  Body& _body;

#ifndef _MSC_VER

  friend ::symphony::internal::legacy::body_with_attrs<Body>
  with_attrs_cpu<>(::symphony::internal::legacy::task_attrs const& attrs, Body&& body);
  friend ::symphony::internal::legacy::body_with_attrs<Body>
  with_attrs_cpu<>(::symphony::internal::legacy::task_attrs const& attrs, Body&& body,
                   std::nullptr_t);

#endif
};

class body_with_attrs_base_gpu { };

#ifdef SYMPHONY_HAVE_GPU

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template <typename Body,
          typename KernelPtr,
          typename... Kargs>
class body_with_attrs_gpu<Body, KernelPtr, Kargs...> :
                                               public body_with_attrs_base_gpu
{

SYMPHONY_GCC_IGNORE_END("-Weffc++");

public:

  typedef internal::function_traits<Body> body_traits;

  typedef typename body_traits::return_type return_type;

  typedef typename KernelPtr::type::parameters kernel_parameters;

  typedef std::tuple<Kargs...> kernel_arguments;

  SYMPHONY_CONSTEXPR task_attrs const& get_attrs() const { return _attrs;}

  SYMPHONY_CONSTEXPR Body& get_body() const { return _body; }

  template<typename ...Args>
  SYMPHONY_CONSTEXPR return_type operator()(Args... args) const {
    return (get_body())(args...);
  }

  KernelPtr& get_gpu_kernel() { return _kernel; }

  kernel_arguments& get_cl_kernel_args() { return _kargs; }
private:
  SYMPHONY_CONSTEXPR body_with_attrs_gpu(task_attrs const& attrs, Body&& body,
                                     KernelPtr kernel, Kargs&&... args):
    _attrs(attrs),
    _body(body),
    _kernel(kernel),
    _kargs(std::tuple<Kargs...>(std::forward<Kargs>(args)...)) {  }

  task_attrs const& _attrs;
  Body& _body;
  KernelPtr _kernel;

  kernel_arguments _kargs;

#ifndef _MSC_VER
  friend ::symphony::internal::legacy::body_with_attrs_gpu<Body, KernelPtr, Kargs...>
  with_attrs_gpu_internal<>(::symphony::internal::legacy::task_attrs const& attrs, Body&& body,
                        KernelPtr kernel, Kargs... args);

  friend ::symphony::internal::legacy::body_with_attrs_gpu<CpuKernelType,
                                     KernelPtr, Kargs...>
  with_attrs_gpu<>(::symphony::internal::legacy::task_attrs const& attrs,
               KernelPtr kernel, Kargs... args);

#endif

};

#endif

template <typename Body, typename Cancel_Handler>
class body_with_attrs<Body, Cancel_Handler>
{
public:

  typedef internal::function_traits<Body> body_traits;

  typedef typename body_traits::return_type return_type;

  typedef internal::function_traits<Cancel_Handler> cancel_handler_traits;

  typedef typename cancel_handler_traits::return_type
    cancel_handler_return_type;

  SYMPHONY_CONSTEXPR task_attrs const& get_attrs() const {
    return _attrs;
  }

  SYMPHONY_CONSTEXPR Body& get_body() const{
    return _body;
  }

  SYMPHONY_CONSTEXPR Cancel_Handler& get_cancel_handler() const {
    return _handler;
  }

  template<typename ...Args>
  SYMPHONY_CONSTEXPR return_type operator()(Args... args) const {
    return (get_body())(args...);
  }

#ifndef _MSC_VER
private:
  constexpr
#endif
  body_with_attrs(task_attrs const& attrs, Body&& body,
                  Cancel_Handler&& handler):
    _attrs(attrs),
    _body(body),
    _handler(handler){}

  task_attrs const& _attrs;
  Body& _body;
  Cancel_Handler& _handler;

#ifndef _MSC_VER
  friend body_with_attrs<Body, Cancel_Handler>
  with_attrs_cpu<>(task_attrs const& attrs, Body&& body,
                   Cancel_Handler&& handler);
#endif

};

#ifdef ONLY_FOR_DOXYGEN
#error The compiler should not see these methods

template <typename Body>
SYMPHONY_CONSTEXPR body_with_attrs<Body>
with_attrs(task_attrs const& attrs, Body&& body);

template <typename Body, typename Cancel_Handler>
SYMPHONY_CONSTEXPR body_with_attrs<Body, Cancel_Handler>
with_attrs(task_attrs const& attrs, Body&& body, Cancel_Handler &&handler);

template<typename KernelPtr, typename... Kargs>
SYMPHONY_CONSTEXPR body_with_attrs_gpu<CpuKernelType, KernelPtr, Kargs...>
with_attrs(task_attrs const& attrs,
           KernelPtr kernel, Kargs... args);

#endif

template <typename Body>
SYMPHONY_CONSTEXPR body_with_attrs<Body>
with_attrs_cpu(task_attrs const& attrs, Body&& body)
{
  return body_with_attrs<Body>(attrs, std::forward<Body>(body));
}

template <typename Body, typename Cancel_Handler>
SYMPHONY_CONSTEXPR body_with_attrs<Body, Cancel_Handler>
with_attrs_cpu(task_attrs const& attrs, Body&& body, Cancel_Handler &&handler)
{
  return body_with_attrs<Body, Cancel_Handler>
    (attrs, std::forward<Body>(body),
     std::forward<Cancel_Handler>(handler));
}

#ifdef SYMPHONY_HAVE_GPU

template<typename Body, typename KernelPtr, typename... Kargs>
SYMPHONY_CONSTEXPR body_with_attrs_gpu<Body, KernelPtr, Kargs...>
with_attrs_gpu_internal(task_attrs const& attrs, Body&& body,
           KernelPtr kernel, Kargs... args)
{
  return body_with_attrs_gpu<Body,
                             KernelPtr,
                             Kargs...> (attrs, std::forward<Body>(body),
                                        kernel,
                                        std::forward<Kargs>(args)...);
}

template<typename KernelPtr, typename... Kargs>
SYMPHONY_CONSTEXPR
body_with_attrs_gpu<CpuKernelType, KernelPtr, Kargs...>
with_attrs_gpu(task_attrs const& attrs,
           KernelPtr kernel, Kargs... args)
{
  return with_attrs_gpu_internal(attrs,
                             std::forward<CpuKernelType>
                                          (default_cpu_kernel),
                             kernel,
                             std::forward<Kargs>(args)...);
}

template<typename Body, typename... Kargs>
struct with_attrs_dispatcher<Body, std::true_type, Kargs...>
{
  static auto dispatch(task_attrs const& attrs, Body&& body, Kargs... args)
    ->decltype(with_attrs_gpu(attrs,
                              std::forward<Body>(body),
                              std::forward<Kargs>(args)...))
  {
    return with_attrs_gpu(attrs,
                          std::forward<Body>(body),
                          std::forward<Kargs>(args)...);
  }
};

#endif

template<typename Body, typename... Kargs>
struct with_attrs_dispatcher<Body, std::false_type, Kargs...>
{
  static auto dispatch(task_attrs const& attrs, Body&& body, Kargs&&... args)
    ->decltype(with_attrs_cpu(attrs,
                              std::forward<Body>(body),
                              std::forward<Kargs>(args)...))
  {
    return with_attrs_cpu(attrs,
                          std::forward<Body>(body),
                          std::forward<Kargs>(args)...);
  }
};

template<typename Body, typename = void>
struct is_gpu_kernel
{
  typedef Body type;
  typedef std::false_type result_type;
};

class gpu_kernel_base { };

template<typename Body>
struct is_gpu_kernel<
         Body,
         typename is_gpu_kernel<void, typename Body::type>::type>
{
  typedef typename Body::type type;
  typedef typename std::is_base_of<internal::legacy::gpu_kernel_base,
                                   typename Body::type>::type result_type;
};

template<typename Body, typename... Kargs>
SYMPHONY_CONSTEXPR auto
with_attrs(task_attrs const& attrs, Body&& body, Kargs&&... args)
  ->decltype(with_attrs_dispatcher<
               Body, typename is_gpu_kernel<
                      typename std::remove_reference<Body>::type>::result_type,
               Kargs...>::dispatch(attrs,
                                   std::forward<Body>(body),
                                   std::forward<Kargs>(args)...))
{  return with_attrs_dispatcher<
               Body, typename is_gpu_kernel<
                      typename std::remove_reference<Body>::type>::result_type,
               Kargs...>::dispatch(attrs,
                                   std::forward<Body>(body),
                                   std::forward<Kargs>(args)...);
}

inline SYMPHONY_CONSTEXPR
bool operator== (symphony::internal::legacy::task_attrs const& a,
                 symphony::internal::legacy::task_attrs const& b) {
  return a._mask == b._mask;
}

inline SYMPHONY_CONSTEXPR
bool operator!= (symphony::internal::legacy::task_attrs const& a,
                 symphony::internal::legacy::task_attrs const& b) {
  return a._mask != b._mask;
}

};
};
};

