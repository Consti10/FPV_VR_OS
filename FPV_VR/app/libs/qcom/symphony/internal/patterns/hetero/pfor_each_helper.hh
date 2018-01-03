// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/hetero/utility.hh>
#include <symphony/internal/buffer/buffertraits.hh>

namespace symphony {

namespace internal {

inline size_t normalize_idx(const size_t idx, const size_t stride)
{
  return (idx / stride + 1) * stride;
}

template <size_t Dims>
struct create_range;

template <>
struct create_range<1>
{
  static symphony::range<1> create(const symphony::range<1>& r,
                                   const size_t first,
                                   const size_t last) {
    return symphony::range<1>(first, last, r.stride(0));
  }
};

template <>
struct create_range<2>
{
  static symphony::range<2> create(const symphony::range<2>& r,
                                   const size_t first,
                                   const size_t last) {
    return symphony::range<2>(first, last, r.stride(0),
                          r.begin(1), r.end(1), r.stride(1));
  }
};

template <>
struct create_range<3>
{
  static symphony::range<3> create(const symphony::range<3>& r,
                                   const size_t first,
                                   const size_t last) {
    return symphony::range<3>(first, last, r.stride(0),
                          r.begin(1), r.end(1), r.stride(1),
                          r.begin(2), r.end(2), r.stride(2));
  }
};

template <size_t Dims>
struct create_gpu_range;

template <>
struct create_gpu_range<1>
{
  static symphony::range<1> create(const symphony::range<1>& r,
                                   const size_t first,
                                   const size_t last) {
    SYMPHONY_UNUSED(r);
    return symphony::range<1>(first, last);
  }
};

template <>
struct create_gpu_range<2>
{
  static symphony::range<2> create(const symphony::range<2>& r,
                                   const size_t first,
                                   const size_t last) {
    return symphony::range<2>(first, last, r.begin(1), r.end(1));
  }
};

template <>
struct create_gpu_range<3>
{
  static symphony::range<3> create(const symphony::range<3>& r,
                                   const size_t first,
                                   const size_t last) {
    return symphony::range<3>(first, last,
                          r.begin(1), r.end(1),
                          r.begin(2), r.end(2));
  }
};

template <size_t Dims>
struct idx2lin;

template <>
struct idx2lin<1>
{
  static size_t transform(const index<1>& idx, const symphony::range<1>& r) {
    SYMPHONY_UNUSED(r);
    return idx[0];
  }
};

template <>
struct idx2lin<2>
{
  static size_t transform(const index<2>& idx, const symphony::range<2>& r) {
    size_t height = r.end(1) - r.begin(1);
    return idx[0] * height + idx[1];
  }
};

template <>
struct idx2lin<3>
{
  static size_t transform(const index<3>& idx, const symphony::range<3>& r) {
    size_t height = r.end(1) - r.begin(1);
    size_t depth  = r.end(2) - r.begin(2);
    return idx[0] * height * depth + idx[1] * depth + idx[2];
  }
};

template <size_t Dims>
struct range_check;

template <>
struct range_check<1>
{
  static bool all_single_stride(const symphony::range<1>& r) {
    return (r.stride(0) == 1);
  }
};

template <>
struct range_check<2>
{
  static bool all_single_stride(const symphony::range<2>& r) {
      return (r.stride(0) == 1) && (r.stride(1) == 1);
  }
};

template <>
struct range_check<3>
{
  static bool all_single_stride(const symphony::range<3>& r) {
    return (r.stride(0) == 1) && (r.stride(1) == 1) && (r.stride(2) == 1);
  }
};

SYMPHONY_CONSTEXPR_CONST size_t invalid_pos = std::numeric_limits<size_t>::max();

#if defined(SYMPHONY_HAVE_OPENCL)
template<size_t Dims, typename KernelType, typename ArgTuple, size_t... Indices>
void gpu_task_launch_wrapper(symphony::group_ptr& g,
                             symphony::task_ptr<void(void)>& gpu_task,
                             const symphony::range<Dims>& range,
                             KernelType&& kernel,
                             ArgTuple& arglist,
                             const symphony::pattern::tuner& tuner,
                             integer_list_gen<Indices...>)
{
  SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");
  SYMPHONY_API_ASSERT(range.dims() >= 1, "Range object must have at least one dimension!");
  gpu_task = symphony::create_task(std::forward<KernelType>(kernel),
                                   range, std::get<Indices-1>(arglist)...);

  if (tuner.has_profile()) {
    (static_cast<gputask_base*>(c_ptr(gpu_task)))->set_exec_time(symphony_get_time_now());
  }

  pattern::utility::extract_input_arenas_for_gpu
    <sizeof...(Indices), decltype(arglist)>::preacquire_input_arenas(c_ptr(gpu_task), arglist);

  c_ptr(gpu_task)->unsafe_enable_non_locking_buffer_acquire();
  g->launch(gpu_task);
}
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

template<size_t Dims>
struct hexagon_task_launch_wrapper;

template<>
struct hexagon_task_launch_wrapper<1> {
  template<typename KernelType, typename ArgTuple, size_t... Indices>
  static void launch(symphony::group_ptr& g,
                     symphony::task_ptr<void(void)>& hexagon_task,
                     const symphony::range<1>& range,
                     KernelType&& kernel,
                     ArgTuple& arglist,
                     const symphony::pattern::tuner& tuner,
                     integer_list_gen<Indices...>)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");
    SYMPHONY_API_ASSERT(range.dims() == 1, "Expect a 1D range object!");
    hexagon_task = symphony::create_task(std::forward<KernelType>(kernel),
                                         range.begin(0), range.end(0),
                                         std::get<Indices-1>(arglist)...);

    if (tuner.has_profile()) {
      (static_cast<hexagontask_return_layer*>(c_ptr(hexagon_task)))->set_exec_time(symphony_get_time_now());
    }

    pattern::utility::extract_input_arenas_for_hexagon
      <sizeof...(Indices), decltype(arglist)>::preacquire_input_arenas(c_ptr(hexagon_task), arglist);

    c_ptr(hexagon_task)->unsafe_enable_non_locking_buffer_acquire();
    g->launch(hexagon_task);
  }
};

template<>
struct hexagon_task_launch_wrapper<2> {
  template<typename KernelType, typename ArgTuple, size_t... Indices>
  static void launch(symphony::group_ptr& g,
                     symphony::task_ptr<void(void)>& hexagon_task,
                     const symphony::range<2>& range,
                     KernelType&& kernel,
                     ArgTuple& arglist,
                     const symphony::pattern::tuner& tuner,
                     integer_list_gen<Indices...>)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");
    SYMPHONY_API_ASSERT(range.dims() == 2, "Expect a 2D range object!");
    hexagon_task = symphony::create_task(std::forward<KernelType>(kernel),
                                         range.begin(0), range.end(0),
                                         range.begin(1), range.end(1),
                                         std::get<Indices-1>(arglist)...);

    if (tuner.has_profile()) {
      (static_cast<hexagontask_return_layer*>(c_ptr(hexagon_task)))->set_exec_time(symphony_get_time_now());
    }

    pattern::utility::extract_input_arenas_for_hexagon
      <sizeof...(Indices), decltype(arglist)>::preacquire_input_arenas(c_ptr(hexagon_task), arglist);

    c_ptr(hexagon_task)->unsafe_enable_non_locking_buffer_acquire();
    g->launch(hexagon_task);
  }
};

template<>
struct hexagon_task_launch_wrapper<3> {
  template<typename KernelType, typename ArgTuple, size_t... Indices>
  static void launch(symphony::group_ptr& g,
                     symphony::task_ptr<void(void)>& hexagon_task,
                     const symphony::range<3>& range,
                     KernelType&& kernel,
                     ArgTuple& arglist,
                     const symphony::pattern::tuner& tuner,
                     integer_list_gen<Indices...>)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");
    SYMPHONY_API_ASSERT(range.dims() == 3, "Expect a 3D range object!");
    hexagon_task = symphony::create_task(std::forward<KernelType>(kernel),
                                         range.begin(0), range.end(0),
                                         range.begin(1), range.end(1),
                                         range.begin(2), range.end(2),
                                         std::get<Indices-1>(arglist)...);

    if (tuner.has_profile()) {
      (static_cast<hexagontask_return_layer*>(c_ptr(hexagon_task)))->set_exec_time(symphony_get_time_now());
    }

    pattern::utility::extract_input_arenas_for_hexagon
      <sizeof...(Indices), decltype(arglist)>::preacquire_input_arenas(c_ptr(hexagon_task), arglist);

    c_ptr(hexagon_task)->unsafe_enable_non_locking_buffer_acquire();
    g->launch(hexagon_task);
  }
};

#endif

struct cpu_kernel_caller{
template<typename UnaryFn, size_t Dims, typename ArgTuple, size_t... Indices>
typename std::enable_if<!pattern::utility::is_symphony_cpu_kernel<typename std::remove_reference<UnaryFn>::type>::value>::type
static cpu_kernel_call_wrapper(UnaryFn&& fn,
                               const symphony::range<Dims>& r,
                               const symphony::pattern::tuner& tuner,
                               ArgTuple& arglist,
                               integer_list_gen<Indices...>
                               )
  {
      fn(r, tuner, std::get<Indices-1>(arglist)...);
  }

template<typename UnaryFn, size_t Dims, typename ArgTuple, size_t... Indices>
typename std::enable_if<pattern::utility::is_symphony_cpu_kernel<typename std::remove_reference<UnaryFn>::type>::value>::type
static cpu_kernel_call_wrapper(UnaryFn&& cpu_k,
                               const symphony::range<Dims>& r,
                               const symphony::pattern::tuner& tuner,
                               ArgTuple& arglist,
                               integer_list_gen<Indices...>
                               )
  {
    auto fn = cpu_k.get_fn();
    fn(r, tuner, std::get<Indices-1>(arglist)...);
  }
};

#if defined(SYMPHONY_HAVE_OPENCL)

template <size_t KPos, size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T, bool CalledWithPK>
struct execute_on_gpu {
  static void kernel_launch(symphony::task_ptr<void(void)>& gpu_task,
                            const symphony::range<Dims>& r,
                            symphony::group_ptr& g,
                            KernelTuple& klist,
                            ArgTuple arglist,

                            symphony::buffer_ptr<T> gpu_local_buffer,
                            const symphony::pattern::tuner& tuner)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");

    auto gpu_kernel = std::get<KPos>(klist);
    std::get<ArgPos>(arglist) = gpu_local_buffer;
    gpu_task_launch_wrapper(g,
                            gpu_task,
                            r,
                            std::forward<decltype(gpu_kernel)>(gpu_kernel),
                            arglist,
                            tuner,
                            typename integer_list<std::tuple_size<ArgTuple>::value>::type());
  }
};

template <size_t KPos, size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T>
struct execute_on_gpu<KPos, ArgPos, Dims, KernelTuple, ArgTuple, T, true>{
  static void kernel_launch(symphony::task_ptr<void(void)>& gpu_task,
                            const symphony::range<Dims>& r,
                            symphony::group_ptr& g,
                            KernelTuple& klist,
                            ArgTuple arglist,
                            symphony::buffer_ptr<T> gpu_local_buffer,
                            const symphony::pattern::tuner& tuner)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");

    auto gpu_kernel = std::get<KPos>(klist);
    std::get<ArgPos>(arglist) = gpu_local_buffer;
    ::symphony::internal::buffer::utility::expand_buffers_in_args_for_gpu<ArgTuple,0,false> parsed_params(arglist);
    auto new_arglist = parsed_params._expanded_args_list;
    gpu_task_launch_wrapper(g,
                            gpu_task,
                            r,
                            std::forward<decltype(gpu_kernel)>(gpu_kernel),
                            new_arglist,
                            tuner,
                            typename integer_list<std::tuple_size<decltype(new_arglist)>::value>::type());
  }
};

template <size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T>
struct execute_on_gpu<invalid_pos, ArgPos, Dims, KernelTuple, ArgTuple, T, true> {
  static void kernel_launch(symphony::task_ptr<void(void)>&,
                            const symphony::range<Dims>&,
                            symphony::group_ptr&,
                            KernelTuple&,
                            ArgTuple,
                            symphony::buffer_ptr<T>,
                            const symphony::pattern::tuner&)
  {}
};

template <size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T>
struct execute_on_gpu<invalid_pos, ArgPos, Dims, KernelTuple, ArgTuple, T, false> {
  static void kernel_launch(symphony::task_ptr<void(void)>&,
                            const symphony::range<Dims>&,
                            symphony::group_ptr&,
                            KernelTuple&,
                            ArgTuple,
                            symphony::buffer_ptr<T>,
                            const symphony::pattern::tuner&)
  {}
};

#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

template <size_t KPos, size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T>
struct execute_on_dsp {
  static void kernel_launch(symphony::task_ptr<void(void)>& hexagon_task,
                            const symphony::range<Dims>& r,
                            symphony::group_ptr& g,
                            KernelTuple& klist,
                            ArgTuple& arglist,
                            symphony::buffer_ptr<T> dsp_local_buffer,
                            const symphony::pattern::tuner& tuner)
  {
    SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group!");
    std::get<ArgPos>(arglist) = dsp_local_buffer;
    auto hexagon_kernel = std::get<KPos>(klist);
    hexagon_task_launch_wrapper<Dims>::
      launch(g, hexagon_task, r, std::forward<decltype(hexagon_kernel)>(hexagon_kernel),
             arglist, tuner, typename integer_list<std::tuple_size<ArgTuple>::value>::type());
  }
};

template <size_t ArgPos, size_t Dims, typename KernelTuple, typename ArgTuple, typename T>
struct execute_on_dsp<invalid_pos, ArgPos, Dims, KernelTuple, ArgTuple, T>{
  static void kernel_launch(symphony::task_ptr<void(void)>&,
                            const symphony::range<Dims>&,
                            symphony::group_ptr&,
                            KernelTuple&,
                            ArgTuple& ,
                            symphony::buffer_ptr<T>,
                            const symphony::pattern::tuner&)
  {}
};
#endif

template <size_t KPos, size_t Dims, typename KernelTuple, typename ArgTuple, bool CalledWithPK>
struct execute_on_cpu {
  static void kernel_launch(const symphony::range<Dims>& r,
                            KernelTuple& klist,
                            ArgTuple& arglist,
                            const symphony::pattern::tuner& tuner)
  {
    auto cpu_kernel = std::get < KPos > (klist);
    cpu_kernel_caller::cpu_kernel_call_wrapper(
    std::forward<decltype(cpu_kernel)>(cpu_kernel), r, tuner, arglist,
                              typename integer_list<std::tuple_size<ArgTuple>::value>::type());
  }
};

template <size_t KPos, size_t Dims, typename KernelTuple, typename ArgTuple>
struct execute_on_cpu<KPos, Dims, KernelTuple, ArgTuple, true>{
  static void kernel_launch(const symphony::range<Dims>& r,
                            KernelTuple& klist,
                            ArgTuple& arglist,
                            const symphony::pattern::tuner& tuner)
  {
      auto cpu_kernel = std::get <KPos> (klist);
      ::symphony::internal::buffer::utility::expand_buffers_in_args_for_cpu<ArgTuple, 0,false> parsed_params(arglist);
      auto new_arglist = parsed_params._expanded_args_list;
      cpu_kernel_caller::cpu_kernel_call_wrapper(
      std::forward<decltype(cpu_kernel)>(cpu_kernel), r, tuner, new_arglist,
                              typename integer_list<std::tuple_size<decltype(new_arglist)>::value>::type());
  }
};

template <size_t Dims, typename KernelTuple, typename ArgTuple>
struct execute_on_cpu<invalid_pos, Dims, KernelTuple, ArgTuple, true>{
  static void kernel_launch(const symphony::range<Dims>&,
                            KernelTuple&,
                            ArgTuple&,
                            const symphony::pattern::tuner&)
  {}
};

template <size_t Dims, typename KernelTuple, typename ArgTuple>
struct execute_on_cpu<invalid_pos, Dims, KernelTuple, ArgTuple, false>{
  static void kernel_launch(const symphony::range<Dims>&,
                            KernelTuple&,
                            ArgTuple&,
                            const symphony::pattern::tuner&)
  {}
};

};
};

