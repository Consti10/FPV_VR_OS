// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/cpu_pfor_each.hh>
#include <symphony/internal/patterns/gpu_pfor_each.hh>
#include <symphony/internal/patterns/hetero/pfor_each_helper.hh>
#include <symphony/internal/pointkernel/pointkernel.hh>

namespace symphony {

namespace pattern {
  template <typename T1, typename T2> class pfor;
};

namespace internal {

template<class InputIterator, typename UnaryFn, typename T>
struct pfor_each_dispatcher;

template<typename UnaryFn>
struct pfor_each_dispatcher<size_t, UnaryFn, std::true_type>
{

  static void dispatch(group* g, size_t first, size_t last,
                       UnaryFn&& fn, const size_t,
                       const symphony::pattern::tuner&) {
    pfor_each_gpu(g, first, last, std::forward<UnaryFn>(fn));
  }

  template<size_t Dims>
  static void dispatch(group* g, const symphony::range<Dims>& r,
                       UnaryFn&& fn, const symphony::pattern::tuner&) {
    for(auto i : r.stride())
      SYMPHONY_API_ASSERT( (i== 1), "GPU ranges must be unit stride");

    symphony::range<Dims> l;
    pfor_each_gpu(g, r, l, std::forward<UnaryFn>(fn));
  }
};

template<typename UnaryFn, typename IterType>
void func_impl(const size_t& idx, IterType first, UnaryFn&& fn, std::true_type)
{
  fn(static_cast<IterType>(idx) + first);
}

template<typename UnaryFn, class IterType>
void func_impl(const size_t& idx, IterType first, UnaryFn&& fn, std::false_type)
{
  fn(idx + first);
}

template<class InputIterator, typename UnaryFn>
struct pfor_each_dispatcher<InputIterator, UnaryFn, std::false_type>
{
  static void dispatch(group* g, InputIterator first,
                       InputIterator last, UnaryFn&& fn, const size_t stride,
                       const symphony::pattern::tuner& t)
  {
    SYMPHONY_API_ASSERT(symphony::internal::callable_object_is_mutable<UnaryFn>::value == false,
                        "Mutable functor is not allowed in symphony patterns!");

    if (t.is_serial()) {
      for (InputIterator idx = first; idx < last; idx += stride) {
        fn(idx);
      }
      return;
    }

    if (t.is_static()) {
      pfor_each_static(g, first, last, std::forward<UnaryFn>(fn), stride, t);
      return;
    }

    const auto fn_transform = [fn, first](size_t i) {
      func_impl(i, first, fn, std::is_integral<InputIterator>());
    };

    pfor_each_dynamic(g, size_t(0), size_t(last - first), fn_transform, stride, t);

  }
};

template<typename UnaryFn>
struct pfor_each_dispatcher<size_t, UnaryFn, std::false_type>
{

  static void dispatch(group* g, size_t first, size_t last,
                       UnaryFn&& fn, const size_t stride,
                       symphony::pattern::tuner const & t) {

    SYMPHONY_API_ASSERT(symphony::internal::callable_object_is_mutable<UnaryFn>::value == false,
                        "Mutable functor is not allowed in symphony patterns!");

    if (t.is_serial()) {
      for (size_t idx = first; idx < last; idx += stride) {
        fn(idx);
      }
    }

    else if (t.is_static()) {
      pfor_each_static(g, first, last, std::forward<UnaryFn>(fn), stride, t);
    }

    else {
      pfor_each_dynamic(g, first, last, std::forward<UnaryFn>(fn), stride, t);
    }
  }

  template<size_t Dims>
  static void dispatch(group* g,
                       const symphony::range<Dims>& r,
                       UnaryFn&& fn,
                       const symphony::pattern::tuner& t) {
   pfor_each_range<Dims, UnaryFn>::pfor_each_range_impl(g, r, std::forward<UnaryFn>(fn), t);
  }
};

template <class InputIterator, typename UnaryFn>
void
pfor_each_internal(group* g, InputIterator first, InputIterator last,
                   UnaryFn&& fn, const size_t stride,
                   const symphony::pattern::tuner& t = symphony::pattern::tuner())
{

  internal::pfor_each_dispatcher<InputIterator, UnaryFn,
    typename std::is_base_of<legacy::body_with_attrs_base_gpu, UnaryFn>::type>::
    dispatch(g, first, last, std::forward<UnaryFn>(fn), stride, t);
}

template<size_t Dims, typename UnaryFn>
void pfor_each_internal(group* g, const symphony::range<Dims>& r,
                        UnaryFn&& fn, const symphony::pattern::tuner& t)
{

  internal::pfor_each_dispatcher<size_t, UnaryFn,
    typename std::is_base_of<legacy::body_with_attrs_base_gpu, UnaryFn>::type>::
    dispatch(g, r, std::forward<UnaryFn>(fn), t);
}

template <class InputIterator, typename UnaryFn>
void
pfor_each(group_ptr group, InputIterator first, InputIterator last,
          UnaryFn&& fn, const size_t stride = 1,
          const symphony::pattern::tuner& t = symphony::pattern::tuner())
{
  auto gptr = c_ptr(group);
  pfor_each_internal(gptr, first, last, std::forward<UnaryFn>(fn), stride, t);
}

template <class InputIterator, typename UnaryFn>
void pfor_each(int, InputIterator, InputIterator, UnaryFn) = delete;

template<size_t Dims, typename UnaryFn>
void pfor_each(group_ptr group, const symphony::range<Dims>& r,
               UnaryFn&& fn, const symphony::pattern::tuner& t)
{
  auto gptr = c_ptr(group);
  pfor_each_internal(gptr, r, std::forward<UnaryFn>(fn), t);
}

template <class InputIterator, typename UnaryFn>
symphony::task_ptr<void()>
pfor_each_async(UnaryFn fn,
                InputIterator first,
                InputIterator last,
                const size_t stride = 1,
                const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto g = create_group();
  auto t = symphony::create_task([g, first, last, fn, stride, tuner]{
      internal::pfor_each(g, first, last, fn, stride, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <size_t Dims, typename UnaryFn>
symphony::task_ptr<void()>
pfor_each_async(UnaryFn fn,
                const symphony::range<Dims>& r,
                const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto g = create_group();
  auto t = symphony::create_task([g, r, fn, tuner]{
      internal::pfor_each(g, r, fn, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <size_t Dims,
          typename KernelTuple, typename ArgTuple,
          typename KernelFirst, typename... KernelRest,
          typename... Args,
          typename Boolean,
          typename Buf_Tuple>
void pfor_each(symphony::pattern::pfor<KernelTuple, ArgTuple>* const p,
               const symphony::range<Dims>& r,
               std::tuple<KernelFirst, KernelRest...>& klist,
               symphony::pattern::tuner& tuner,
               const Boolean called_with_pointkernel,
               Buf_Tuple&& buf_tup,
               Args&&... args)
{
  SYMPHONY_UNUSED(buf_tup);
  bool have_gpu = false;
  bool have_hexagon = false;

#if defined(SYMPHONY_HAVE_OPENCL)
  have_gpu = true;
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  have_hexagon = true;
#endif

  size_t first = r.begin(0);
  size_t last  = r.end(0);

  auto arg_list = std::make_tuple(std::forward<Args>(args)...);

  using kernel_type = std::tuple<KernelFirst, KernelRest...>;
  using arg_type    = decltype(arg_list);
  using idx_type    = size_t;
  using pct_type    = float;
  using time_type   = uint64_t;

  time_type cpu_exec_begin = 0, cpu_exec_end = 0;
  time_type cpu_exec_time = 0;

  SYMPHONY_CONSTEXPR_CONST bool is_called_with_pointkernel = called_with_pointkernel;

  SYMPHONY_CONSTEXPR_CONST idx_type out_idx = pattern::utility::extract_output_buf<sizeof...(Args), decltype(arg_list)>::value;
  auto output = std::get<out_idx>(arg_list);

  using buf_type    = typename pattern::utility::buffer_data_type<decltype(output)>::data_type;

#if defined(SYMPHONY_HAVE_OPENCL) || defined(SYMPHONY_HAVE_QTI_HEXAGON)
  using bufptr_type = symphony::buffer_ptr<buf_type>;
#endif

  size_t klist_size = std::tuple_size<kernel_type>::value;
  SYMPHONY_API_ASSERT(klist_size > 0 && klist_size <= 3, "Wrong number of kernels provided!");

  if (tuner.is_serial()) {
    const size_t ck_idx = symphony::internal::pattern::utility::cpu_kernel_pos<kernel_type>::pos;
    SYMPHONY_API_ASSERT(ck_idx != invalid_pos, "Serial tuner requires a valid CPU kernel.");
    execute_on_cpu<ck_idx, Dims, kernel_type, arg_type, is_called_with_pointkernel>::
      kernel_launch(r, klist, arg_list, tuner);
    return ;
  }

#if defined(SYMPHONY_HAVE_OPENCL) || defined(SYMPHONY_HAVE_QTI_HEXAGON)
  idx_type size  = r.linearized_distance();
  if (size != output.size()) {
    SYMPHONY_FATAL("Iteration space in heterogeneous pfor_each must match output buffer size!");
  }
  idx_type nd_offset = size / (last - first);
#endif

  idx_type stride = r.stride(0);

  SYMPHONY_CONSTEXPR_CONST size_t ck_idx = pattern::utility::cpu_kernel_pos<kernel_type>::pos;
  SYMPHONY_CONSTEXPR_CONST size_t gk_idx = pattern::utility::gpu_kernel_pos<kernel_type>::pos;
  SYMPHONY_CONSTEXPR_CONST size_t hk_idx = pattern::utility::hexagon_kernel_pos<kernel_type>::pos;

  if (tuner.has_profile()) {
    SYMPHONY_API_ASSERT(ck_idx != invalid_pos,
                        "Auto tuning requires a valid CPU kernel to be profiled as baseline.");

    SYMPHONY_API_ASSERT(last-first >= 3,
                        "Auto tuning is impossible due to insufficient number of iterations.");

    if (ck_idx != invalid_pos && gk_idx != invalid_pos && hk_idx != invalid_pos) {
      tuner.set_cpu_load(33).set_gpu_load(33).set_hexagon_load(34);
    } else if (ck_idx != invalid_pos && gk_idx != invalid_pos) {
      tuner.set_cpu_load(50).set_gpu_load(50);
    } else if (ck_idx != invalid_pos && hk_idx != invalid_pos) {
      tuner.set_cpu_load(50).set_hexagon_load(50);
    } else {
      tuner.set_cpu_load(100);
    }
  }

  const idx_type cpu_load = tuner.get_cpu_load();
  const idx_type gpu_load = tuner.get_gpu_load();
  const idx_type dsp_load = tuner.get_hexagon_load();

  SYMPHONY_API_ASSERT(cpu_load + gpu_load + dsp_load == 100,
                      "Incorrect load setting across devices!");
  SYMPHONY_API_ASSERT(!(cpu_load > 0 && ck_idx == invalid_pos),
                      "CPU: kernel and tuner load mismatch!");
  SYMPHONY_API_ASSERT(!(gpu_load > 0 && gk_idx == invalid_pos),
                      "GPU: kernel and tuner load mismatch!");
  SYMPHONY_API_ASSERT(!(dsp_load > 0 && hk_idx == invalid_pos),
                      "Hexagon: Kernel and tuner load mismatch!");
  SYMPHONY_API_ASSERT(!(gpu_load > 0 && !have_gpu),
                      "Must use SYMPHONY_HAVE_OPENCL to dispatch to GPU!");
  SYMPHONY_API_ASSERT(!(dsp_load > 0 && !have_hexagon),
                      "Must use SYMPHONY_HAVE_QTI_HEXAGON to dispatch to DSP!");

  symphony::internal::executor_device_bitset eds;

  eds.add(executor_device::cpu);
  if (gpu_load > 0)

    eds.add(executor_device::gpucl);
  if (dsp_load > 0)
    eds.add(executor_device::hexagon);

#if defined(SYMPHONY_HAVE_OPENCL) && defined(SYMPHONY_HAVE_QTI_HEXAGON)
  static SYMPHONY_CONSTEXPR_CONST size_t num_devices = 3;
#elif defined(SYMPHONY_HAVE_OPENCL) || defined(SYMPHONY_HAVE_QTI_HEXAGON)
  static SYMPHONY_CONSTEXPR_CONST size_t num_devices = 2;
#else
  static SYMPHONY_CONSTEXPR_CONST size_t num_devices = 1;
#endif
  SYMPHONY_CONSTEXPR_CONST size_t num_buffer = pattern::utility::get_num_buffer<arg_type>::value;
  buffer_acquire_set<num_buffer, true, num_devices> bas;
  pattern::utility::add_buffer_in_args<sizeof...(Args), num_buffer, decltype(arg_list), decltype(bas)>::add_buffer(bas, arg_list);

  size_t uid = 0;

  auto& output_base_ptr = reinterpret_cast<symphony::internal::buffer_ptr_base&>(std::get<out_idx>(arg_list));
  auto output_bs = symphony::internal::c_ptr(symphony::internal::buffer_accessor::get_bufstate(output_base_ptr));

  symphony::internal::override_device_sets<true, 1> ods;
  ods.register_override_device_set(output_bs, {symphony::internal::executor_device::cpu});

  bas.acquire_buffers(&uid, eds, false, nullptr, &ods);
  SYMPHONY_API_ASSERT(bas.acquired(), "Buffer acquire failure!");

  pct_type gpu_fraction = pct_type(gpu_load) / 100.00;
  pct_type dsp_fraction = pct_type(dsp_load) / 100.00;

  auto gpu_num_iters = idx_type((last - first) * gpu_fraction);
  auto dsp_num_iters = idx_type((last - first) * dsp_fraction);

  idx_type gpu_begin = first;
  idx_type gpu_end = gpu_begin + gpu_num_iters;

  idx_type dsp_begin = gpu_end % stride == 0 ? gpu_end : normalize_idx(gpu_end, stride);
  idx_type dsp_end = dsp_begin + dsp_num_iters;

  idx_type cpu_begin = dsp_end % stride == 0 ? dsp_end : normalize_idx(dsp_end, stride);
  idx_type cpu_end = last;

  int cpu_num_iters = cpu_end - cpu_begin;

  auto* output_arena = bas.find_acquired_arena(output, executor_device::cpu);
  SYMPHONY_INTERNAL_ASSERT(output_arena != nullptr, "Invalid arena pointer!");

  auto* optr = static_cast<buf_type*>(arena_storage_accessor::get_ptr(output_arena));
  SYMPHONY_INTERNAL_ASSERT(optr != nullptr, "Unexpected null storage in acquired arena!");

#if defined(SYMPHONY_HAVE_OPENCL)
  bufptr_type gpu_buffer = std::get<0>(buf_tup);;
  buf_type* gpu_buffer_ptr = nullptr;
  idx_type gpu_num_bytes_to_copy = 0;
  if (gpu_num_iters > 0) {
    gpu_num_bytes_to_copy = gpu_num_iters * nd_offset * sizeof(buf_type);

    if (!range_check<Dims>::all_single_stride(r)) {
      gpu_buffer.w_invalidate();
      SYMPHONY_API_ASSERT(gpu_buffer.host_data() != nullptr,
                          "gpu privatized buffer is not host accessible!");
      gpu_buffer_ptr = static_cast<buf_type*>(gpu_buffer.host_data());
      memcpy(gpu_buffer_ptr, optr, gpu_num_bytes_to_copy);
    }
  }
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  std::vector<bufptr_type> dsp_buffer_vec = std::get<1>(buf_tup);;

  std::vector<buf_type*> dsp_buffer_ptr(get_num_dsp_threads());
  std::vector<std::pair<size_t, size_t>> dsp_info_vec;
#endif

  auto g = symphony::create_group();

#if defined(SYMPHONY_HAVE_OPENCL)

  symphony::task_ptr<void(void)> gpu_task;
  if (gpu_num_iters > 0) {
    symphony::range<Dims> gpu_range = create_gpu_range<Dims>::create(r, gpu_begin, gpu_end);
    execute_on_gpu<gk_idx, out_idx, Dims, kernel_type, arg_type, buf_type, is_called_with_pointkernel>::
      kernel_launch(gpu_task, gpu_range, g, klist, arg_list, gpu_buffer, tuner);
  }
#endif

  auto arg_list_for_hexagon_task = arg_list;

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

  symphony::task_ptr<void(void)> hexagon_task;
  if (dsp_num_iters > 0){
    idx_type block = dsp_num_iters / get_num_dsp_threads();
    idx_type extra = dsp_num_iters % get_num_dsp_threads();

    size_t iter_cnt = tuner.is_serial() ? 1 : get_num_dsp_threads();
    for (size_t i = 0; i < iter_cnt; ++i) {
      idx_type begin = dsp_begin + i * block;
      idx_type end = (i == get_num_dsp_threads() - 1) ? (dsp_begin + (i + 1) * block + extra) :
                                                   (dsp_begin + (i + 1) * block);

      dsp_info_vec.push_back(std::make_pair((begin * nd_offset),
                                            ((end - begin) * nd_offset * sizeof(buf_type))
                                           ));

      if (!range_check<Dims>::all_single_stride(r)) {
        dsp_buffer_vec[i].w_invalidate();
        SYMPHONY_API_ASSERT(dsp_buffer_vec[i].host_data() != nullptr,
                            "hexagon privatized buffer is not host accessible!");
        dsp_buffer_ptr[i] = static_cast<buf_type*>(dsp_buffer_vec[i].host_data());
        memcpy(dsp_buffer_ptr[i] + dsp_info_vec[i].first,
               optr + dsp_info_vec[i].first,
               dsp_info_vec[i].second);
      }
      symphony::range<Dims> hexagon_range = create_range<Dims>::create(r, begin, end);

      execute_on_dsp<hk_idx, out_idx, Dims, kernel_type, arg_type, buf_type>::
        kernel_launch(hexagon_task, hexagon_range, g, klist, arg_list_for_hexagon_task, dsp_buffer_vec[i], tuner);
    }
  }
#endif

  if (cpu_num_iters > 0) {
    symphony::range<Dims> cpu_range = create_range<Dims>::create(r, cpu_begin, cpu_end);
    if (tuner.has_profile()) {
      cpu_exec_begin = symphony_get_time_now();
    }
    execute_on_cpu<ck_idx, Dims, kernel_type, arg_type, is_called_with_pointkernel>::
      kernel_launch(cpu_range, klist, arg_list, tuner);
    if (tuner.has_profile()) {
      cpu_exec_end = symphony_get_time_now();
      cpu_exec_time = cpu_exec_end - cpu_exec_begin;
      p->set_cpu_task_time(cpu_exec_time / 1000);
    }
  }

  g->wait_for();

#if defined(SYMPHONY_HAVE_OPENCL)
  if (gpu_num_iters > 0) {

    gpu_buffer.ro_sync();
    SYMPHONY_API_ASSERT(gpu_buffer.host_data() != nullptr,
                        "gpu privatized buffer is not host accessible!");
    gpu_buffer_ptr = static_cast<buf_type*>(gpu_buffer.host_data());
    memcpy(optr, gpu_buffer_ptr, gpu_num_bytes_to_copy);
  }
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  if (dsp_num_iters > 0) {

    size_t iter_cnt = tuner.is_serial() ? 1 : get_num_dsp_threads();

    for (size_t i = 0; i < iter_cnt; ++i) {

      dsp_buffer_vec[i].ro_sync();
      SYMPHONY_API_ASSERT(dsp_buffer_vec[i].host_data() != nullptr,
                          "hexagon privatized buffer is not host accessible!");
      dsp_buffer_ptr[i] = static_cast<buf_type*>(dsp_buffer_vec[i].host_data());
      memcpy(optr + dsp_info_vec[i].first,
             dsp_buffer_ptr[i] + dsp_info_vec[i].first,
             dsp_info_vec[i].second);
    }
  }
#endif

  bas.release_buffers(&uid);

#if defined(SYMPHONY_HAVE_OPENCL)
  uint64_t gpu_exec_time = 0;
  if (gpu_task != nullptr) {
    gpu_exec_time = static_cast<gputask_base*>(c_ptr(gpu_task))->get_exec_time();
    p->set_gpu_task_time(gpu_exec_time / 1000);
  }
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
  uint64_t dsp_exec_time = 0;
  if (hexagon_task != nullptr) {
    dsp_exec_time = static_cast<hexagontask_return_layer*>(c_ptr(hexagon_task))->get_exec_time();
    p->set_hexagon_task_time(dsp_exec_time / 1000);
  }
#endif

  if (tuner.has_profile()) {

    uint64_t tscale = 1e3;
    double   profile_threshold = 10.0;
    double   epsilon = 1e-3;
    SYMPHONY_UNUSED(epsilon);

    SYMPHONY_API_ASSERT(cpu_num_iters > 0, "Profile: Unexpected number of CPU iterations!");
    double cput = static_cast<double>(cpu_exec_time / tscale);
    if (cput < profile_threshold) {
      SYMPHONY_ALOG("Warning: CPU profile in tiny granularity, abort profiling...");
      return ;
    }

#if defined(SYMPHONY_HAVE_OPENCL)
    if (gpu_load > 0 && gk_idx != invalid_pos) {
      double gpu_relative_time = static_cast<double>(gpu_exec_time) / static_cast<double>(cpu_exec_time);
      double throughput_coefficient_gpu = 0;
      SYMPHONY_API_ASSERT(gpu_num_iters > 0, "Profile: Unexpected number of GPU iterations!");
      throughput_coefficient_gpu = static_cast<double>(cpu_num_iters) / static_cast<double>(gpu_num_iters);
      if (gpu_relative_time < epsilon) {
        SYMPHONY_ALOG("Warning: GPU profile in tiny granularity, abort profiling...");
      } else {
        p->set_gpu_profile(gpu_relative_time * throughput_coefficient_gpu);
      }
    }
#endif

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)
    if (dsp_load > 0 && hk_idx != invalid_pos) {
      double dsp_relative_time = static_cast<double>(dsp_exec_time) / static_cast<double>(cpu_exec_time);
      double throughput_coefficient_dsp = 0;
      SYMPHONY_API_ASSERT(dsp_num_iters > 0, "Profile: Unexpected number of DSP iterations!");
      throughput_coefficient_dsp = static_cast<double>(cpu_num_iters) / static_cast<double>(dsp_num_iters);
      if (dsp_relative_time < epsilon) {
        SYMPHONY_ALOG("Warning: Hexagon profile in tiny granularity, abort profiling...");
      } else {
        p->set_dsp_profile(dsp_relative_time * throughput_coefficient_dsp);
      }
    }
#endif

    p->add_run();

  }

}

template <typename KernelTuple, typename ArgTuple, size_t Dims, size_t... Indices>
void pfor_each_run_helper(symphony::pattern::pfor<KernelTuple, ArgTuple>* const p,
                          const symphony::range<Dims>& r,
                          KernelTuple& klist,
                          symphony::pattern::tuner& t,
                          symphony::internal::integer_list_gen<Indices...>,
                          ArgTuple& atpl)
{

  auto num_output_buf = symphony::internal::pattern::utility::num_output_buffer_in_tuple<ArgTuple>::value;
  SYMPHONY_API_ASSERT(num_output_buf == 1, "Requires one and only one output buffer in parameter list.");

  SYMPHONY_CONSTEXPR_CONST size_t out_idx =
    symphony::internal::pattern::utility::extract_output_buf<std::tuple_size<ArgTuple>::value, ArgTuple>::value;
  SYMPHONY_API_ASSERT(out_idx != symphony::internal::pattern::utility::invalid_pos,
                      "Requires at least one output buffer in the argument list!");
  auto output = std::get<out_idx>(atpl);

#if defined(SYMPHONY_HAVE_OPENCL) || defined(SYMPHONY_HAVE_QTI_HEXAGON)
  using buf_type    = typename symphony::internal::pattern::utility::buffer_data_type<decltype(output)>::data_type;
#endif
  size_t buffer_size = r.linearized_distance();
  SYMPHONY_UNUSED(buffer_size);
#ifdef SYMPHONY_HAVE_OPENCL
  auto gpu_local_buffer = symphony::create_buffer<buf_type>(buffer_size, symphony::buffer_mode::relaxed);
#endif
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
  std::vector<symphony::buffer_ptr<buf_type>> hexagon_buffer_vec(get_num_dsp_threads());
  for(size_t i = 0; i < get_num_dsp_threads(); i++){
    hexagon_buffer_vec[i] = symphony::create_buffer<buf_type>(buffer_size, symphony::buffer_mode::relaxed, {symphony::hexagon});
  }
#endif

  auto local_buffer_tuple = std::make_tuple(
#if defined(SYMPHONY_HAVE_OPENCL) && defined(SYMPHONY_HAVE_QTI_HEXAGON)
    gpu_local_buffer, hexagon_buffer_vec
#elif defined(SYMPHONY_HAVE_OPENCL)
    gpu_local_buffer
#elif defined(SYMPHONY_HAVE_QTI_HEXAGON)
    hexagon_buffer_vec
#else

#endif
  );

  symphony::internal::pfor_each(p, r, klist, t, symphony::internal::buffer::utility::constant<bool, false>(),
                                std::move(local_buffer_tuple),
                                std::get<Indices-1>(std::forward<ArgTuple>(atpl))...);
}

template <typename RT, typename... PKArgs, typename ArgTuple, size_t Dims, size_t... Indices>
void pfor_each_run_helper(symphony::pattern::pfor<pointkernel::pointkernel<RT,PKArgs...>, ArgTuple>* const p,
                          const symphony::range<Dims>& r,
                          pointkernel::pointkernel<RT,PKArgs...>& pk,
                          symphony::pattern::tuner& t,
                          symphony::internal::integer_list_gen<Indices...>,
                          ArgTuple& atpl)
{

  auto num_output_buf = symphony::internal::pattern::utility::num_output_buffer_in_tuple<ArgTuple>::value;
  SYMPHONY_API_ASSERT(num_output_buf == 1, "Requires one and only one output buffer in parameter list.");

  auto klist = std::make_tuple(pk._cpu_kernel
#ifdef SYMPHONY_HAVE_OPENCL
                               ,pk._gpu_kernel
#endif
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
                               ,pk._hexagon_kernel
#endif
                                 );

  size_t buffer_size = r.linearized_distance();
  SYMPHONY_UNUSED(buffer_size);
#ifdef SYMPHONY_HAVE_OPENCL
  if(t.get_gpu_load() > 0){
    auto gpu_buffer = pk._gpu_local_buffer;
    using gpu_buffer_base_type = typename symphony::internal::buffer::utility::buffertraits<decltype(gpu_buffer)>::element_type;
    if(gpu_buffer == nullptr || gpu_buffer.size() < buffer_size){

      gpu_buffer = symphony::create_buffer<gpu_buffer_base_type>(buffer_size, symphony::buffer_mode::relaxed);

      pk._gpu_local_buffer = gpu_buffer;
    }
  }
#endif
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
  if(t.get_hexagon_load() > 0) {
    auto hexagon_buffer_vec = pk._hexagon_local_buffer_vec;
    using hexagon_buffer_base_type = typename symphony::internal::buffer::utility::buffertraits<
                                       typename std::remove_reference<decltype(hexagon_buffer_vec[0])>::type>::element_type;
    for(size_t i = 0; i < hexagon_buffer_vec.size(); i++){
      auto hexagon_buffer = hexagon_buffer_vec[i];
      if(hexagon_buffer == nullptr || hexagon_buffer.size() < buffer_size) {

        hexagon_buffer = symphony::create_buffer<hexagon_buffer_base_type>(buffer_size,
                                                                           symphony::buffer_mode::relaxed,
                                                                           {symphony::hexagon});
        pk._hexagon_local_buffer_vec[i] = hexagon_buffer;
      }
    }
  }
#endif

  auto local_buffer_tuple = std::make_tuple(
#if defined(SYMPHONY_HAVE_OPENCL) && defined(SYMPHONY_HAVE_QTI_HEXAGON)
    pk._gpu_local_buffer, pk._hexagon_local_buffer_vec
#elif defined(SYMPHONY_HAVE_OPENCL)
    pk._gpu_local_buffer
#elif defined(SYMPHONY_HAVE_QTI_HEXAGON)
    pk._hexagon_local_buffer_vec
#else

#endif
  );
  symphony::internal::pfor_each(p, r, klist, t, symphony::internal::buffer::utility::constant<bool, true>(),
                                std::move(local_buffer_tuple),
                                std::get<Indices-1>(std::forward<ArgTuple>(atpl))...);
}

};
};
