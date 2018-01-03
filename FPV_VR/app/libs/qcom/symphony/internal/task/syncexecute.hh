// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/task/syncexecuteinternal.hh>

namespace symphony {
namespace internal {

template<typename Code, typename ...Args>
void execute_function(Code&& code, Args&&... args) {

  using params_tuple = typename internal::function_traits<Code>::args_tuple;
  size_t constexpr num_buffers = symphony::internal::num_buffer_ptrs_in_tuple<params_tuple>::value;

  symphony::internal::buffer_acquire_set<num_buffers> bas;

  internal::add_buffers<decltype(bas), params_tuple, Args...>(bas, std::forward<Args>(args)...);

  int dummy_requestor = 0;
  auto requestor = static_cast<void *>(&dummy_requestor);

  bas.blocking_acquire_buffers(requestor, {symphony::internal::executor_device::cpu});

  code(std::forward<Args>(args)...);
  bas.release_buffers(requestor);
}

#ifdef SYMPHONY_HAVE_GPU

template<typename GPUKernel, typename ...CallArgs>
void execute_gpu(GPUKernel const& gk, CallArgs&&... args) {
  size_t constexpr num_buffers = symphony::internal::num_buffer_ptrs_in_args<CallArgs...>::value;
  symphony::internal::buffer_acquire_set<num_buffers> bas;

SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing");
  auto const& int_gk = reinterpret_cast<typename GPUKernel::parent const&>(gk);
SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing");
  int dummy_requestor = 0;
  void* requestor = static_cast<void*>(&dummy_requestor);
  int_gk.execute_with_global_range(symphony::internal::executor_construct(requestor, false, false),
                                   bas,
                                   true,
                                   true,
                                   std::forward<CallArgs>(args)...);
}

template<typename GPUKernel, size_t Dims, typename ...CallArgs>
struct executor;

template<typename GPUKernel, size_t Dims, typename ...CallArgs>
executor<GPUKernel, Dims, CallArgs...>
create_executor(GPUKernel const& gk,
                symphony::range<Dims> const& global_range,
                symphony::range<Dims> const& local_range,
                CallArgs&&... args)
{
  return executor<GPUKernel, Dims, CallArgs...>(gk, global_range, local_range, std::forward<CallArgs>(args)...);
}

template<typename ...Executors>
void execute_seq(Executors const&... executors) {
  std::tuple<Executors const&...> tpe( static_cast<Executors const&>(executors)... );
  execute_seq_enumerator<sizeof...(Executors), std::tuple<Executors const&...>>::
       expand( static_cast<decltype(tpe) const&>(tpe) );
}

template<typename ...Executors>
symphony::task_ptr<> create_task(Executors const&... executors) {

  std::tuple<Executors...> tpe(const_cast<Executors&>(executors)...);
  auto ck = symphony::create_cpu_kernel([tpe]{ apply_tuple(execute_seq<Executors...>, tpe); });
  ck.set_big();
  return symphony::create_task(ck);
}

#endif

};
};
