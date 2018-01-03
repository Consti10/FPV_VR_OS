// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <tuple>

namespace symphony {
namespace internal {

template<typename BAS, typename Param, typename Arg, bool IsBufferPtr>
struct buffer_adder_helper {
  buffer_adder_helper(BAS&, Arg&&) {

  }
};

template<typename BAS, typename Param, typename Arg>
struct buffer_adder_helper<BAS, Param, Arg, true> {
  buffer_adder_helper(BAS& bas, Arg&& arg) {
    static_assert(is_api20_buffer_ptr<Arg>::value, "argument was expected to be a buffer_ptr");

    if(arg == nullptr)
      return;

    bas.add(arg,
            std::conditional< is_const_buffer_ptr<Param>::value,
                              std::integral_constant<bufferpolicy::action_t, bufferpolicy::acquire_r>,
                              std::integral_constant<bufferpolicy::action_t, bufferpolicy::acquire_rw> >::type::value);
  }
};

template<typename BAS, typename Param, typename Arg>
struct buffer_adder {
  buffer_adder(BAS& bas, Arg&& arg) {
    buffer_adder_helper<BAS, Param, Arg, is_api20_buffer_ptr<Param>::value> bah(bas, std::forward<Arg>(arg));
  }
};

template<typename BAS, typename ParamsTuple, size_t index, bool no_more_args, typename ...Args>
struct add_buffers_helper;

template<typename BAS, typename ParamsTuple, size_t index, typename Arg, typename ...Args>
struct add_buffers_helper<BAS, ParamsTuple, index, false, Arg, Args...> {
  static void add(BAS& bas, Arg&& arg, Args&&... args) {
    bool constexpr next_no_more_args = (sizeof...(Args) == 0);
    buffer_adder<BAS, typename std::tuple_element<index, ParamsTuple>::type, Arg> ba(bas, std::forward<Arg>(arg));
    add_buffers_helper<BAS, ParamsTuple, index+1, next_no_more_args, Args...>::add(bas, std::forward<Args>(args)...);
  }
};

template<typename BAS, typename ParamsTuple, size_t index, typename ...Args>
struct add_buffers_helper<BAS, ParamsTuple, index, true, Args...> {
  static void add(BAS& , Args&&... ) {
  }
};

template<typename BAS, typename ParamsTuple, typename ...Args>
void add_buffers(BAS& bas, Args&&... args) {
  bool constexpr no_more_args = (sizeof...(Args) == 0);
  add_buffers_helper<BAS, ParamsTuple, 0, no_more_args, Args...>::add(bas, std::forward<Args>(args)...);
}

#ifdef SYMPHONY_HAVE_GPU

template<typename GPUKernel, size_t Dims, typename ...CallArgs>
struct executor {
  using gpukernel_parent = typename GPUKernel::parent;
  using args_tuple = std::tuple<CallArgs...>;
  using params_tuple = typename GPUKernel::parent::params_tuple;

  GPUKernel  _gk;
  typename std::enable_if<std::tuple_size<args_tuple>::value == std::tuple_size<params_tuple>::value,
                          args_tuple>::type _args_tuple;
  symphony::range<Dims> _global_range;
  symphony::range<Dims> _local_range;

  executor(GPUKernel const& gk,
           symphony::range<Dims> const& global_range,
           symphony::range<Dims> const& local_range,
           CallArgs&&... args) :
    _gk(gk),
    _args_tuple(std::forward<CallArgs>(args)...),
    _global_range(global_range),
    _local_range(local_range)
  {
    SYMPHONY_API_ASSERT(_gk.is_cl(),
                        "Currently only OpenCL kernels are supported by the blocking execution API");
  }
};

template<typename BAS, typename Executor>
struct process_executor_buffers;

template<typename BAS, typename GPUKernel, size_t Dims, typename ...CallArgs>
struct process_executor_buffers<BAS, executor<GPUKernel, Dims, CallArgs...>> {

  using executor_type = executor<GPUKernel, Dims, CallArgs...>;

  static void process(symphony::internal::executor_construct const& exec_cons,
                      BAS& bas,
                      executor<GPUKernel, Dims, CallArgs...> const& exec,
                      bool add_buffers,
                      bool perform_launch)
  {
    SYMPHONY_DLOG("exec=%p add_buffers=%d perform_launch=%d",
                  static_cast<void const*>(&exec),
                  static_cast<int>(add_buffers),
                  static_cast<int>(perform_launch));
    SYMPHONY_INTERNAL_ASSERT(exec_cons.is_blocking(), "Must not be a task");
    SYMPHONY_INTERNAL_ASSERT(exec_cons.is_bundled(), "Must perform a bundled execution via this mechanism");

    auto f = [&](CallArgs&& ...args)
    {
SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing");
      auto const& int_gk = reinterpret_cast<typename executor_type::gpukernel_parent const&>(exec._gk);
SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing");
      int_gk.execute_with_global_and_local_ranges(exec_cons,
                                                  bas,
                                                  add_buffers,
                                                  perform_launch,
                                                  exec._global_range,
                                                  exec._local_range,
                                                  std::forward<CallArgs>(args)...);
    };

    apply_tuple(f, exec._args_tuple);
  }
};

template<typename BAS, typename TP_P_EC_EX, size_t execute_index, bool Finished>
struct process_executors_in_sequence_helper {
  static void process(BAS& bas,
                      TP_P_EC_EX const& tp_p_ec_ex,
                      bool add_buffers,
                      bool perform_launch)
  {
    auto const& p_ec_ex = std::get<execute_index>(tp_p_ec_ex);
    SYMPHONY_INTERNAL_ASSERT(p_ec_ex.first  != nullptr, "null executor_construct");
    SYMPHONY_INTERNAL_ASSERT(p_ec_ex.second != nullptr, "null executor");
    auto const& exec_cons = *(p_ec_ex.first);
    auto const& executor  = *(p_ec_ex.second);

    using Executor = typename std::remove_cv<typename std::remove_reference<decltype(executor)>::type>::type;
    process_executor_buffers<BAS, Executor>::process(exec_cons, bas, executor, add_buffers, perform_launch);

    bool constexpr finished = (std::tuple_size<TP_P_EC_EX>::value == execute_index+1);
    process_executors_in_sequence_helper<BAS, TP_P_EC_EX, execute_index+1, finished>::process(bas,
                                                                                              tp_p_ec_ex,
                                                                                              add_buffers,
                                                                                              perform_launch);
  }
};

template<typename BAS, typename TP_P_EC_EX, size_t execute_index>
struct process_executors_in_sequence_helper<BAS, TP_P_EC_EX, execute_index, true> {
  static void process(BAS& ,
                      TP_P_EC_EX const& ,
                      bool ,
                      bool )
  {}
};

template<typename BAS, typename TP_P_EC_EX>
void process_executors_in_sequence(BAS& bas,
                                   TP_P_EC_EX const& tp_p_ec_ex,
                                   bool add_buffers,
                                   bool perform_launch)
{
  bool constexpr finished = (std::tuple_size<TP_P_EC_EX>::value == 0);
  process_executors_in_sequence_helper<BAS, TP_P_EC_EX, 0, finished>::process(bas,
                                                                              tp_p_ec_ex,
                                                                              add_buffers,
                                                                              perform_launch);

}

template<typename ...Executors>
void execute_seq_bundle_dispatch(std::pair<size_t, Executors const&> const&... indexed_executors) {
  using dyn_sized_bas = symphony::internal::buffer_acquire_set<0, false>;

  int dummy_requestor = 0;
  void* requestor = static_cast<void*>(&dummy_requestor);

  dyn_sized_bas bas;

  symphony::internal::executor_construct const non_last_exec_cons(requestor, true, false);
  symphony::internal::executor_construct const last_exec_cons(requestor, true, true);

  auto const& tp_p_ec_ex = std::make_tuple( std::make_pair<symphony::internal::executor_construct const*,
                                                           Executors const*>
                                                          ( (indexed_executors.first + 1 == sizeof...(Executors) ?
                                                             &last_exec_cons : &non_last_exec_cons),
                                                            &(indexed_executors.second) )... );

  process_executors_in_sequence(bas, tp_p_ec_ex, true, false);

  bas.blocking_acquire_buffers(requestor, {symphony::internal::executor_device::gpucl});
  if(!bas.acquired())
    SYMPHONY_FATAL("Failed to acquire arenas");

  process_executors_in_sequence(bas, tp_p_ec_ex, false, true);

}

template<size_t prev_index, typename TPE, typename ...IndexedExecutors>
struct execute_seq_enumerator {
  static void expand(TPE const& tp, IndexedExecutors const&... ies) {
    using executor_type = typename std::tuple_element<prev_index-1, TPE>::type;
    execute_seq_enumerator<prev_index-1, TPE, std::pair<size_t, executor_type const&>, IndexedExecutors...>::
         expand(tp,
                std::make_pair<size_t, executor_type const&>(prev_index-1,
                                                             static_cast<executor_type const&>(std::get<prev_index-1>(tp))),
                static_cast<IndexedExecutors const&>(ies)... );
  }
};

template<typename TPE, typename ...IndexedExecutors>
struct execute_seq_enumerator<0, TPE, IndexedExecutors...> {
  static void expand(TPE const& , IndexedExecutors const&... ies) {
    execute_seq_bundle_dispatch( static_cast<IndexedExecutors const&>(ies)... );
  }
};

#endif

};
};
