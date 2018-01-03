// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

#include <string>
#include <tuple>
#include <typeinfo>
#include <type_traits>

#include <symphony/exceptions.hh>
#include <symphony/taskptr.hh>

#include <symphony/internal/task/hexagontraits.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/demangler.hh>
#include <symphony/internal/util/strprintf.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {

template<typename ...Stuff> class task_ptr;

namespace internal {

class group;
class task_bundle_dispatch;

class hexagontask_return_layer: public task {

public:

  uint64_t get_exec_time() const {
    return _exec_time;
  }

  void set_exec_time(uint64_t elapsed_time) {
    _exec_time = elapsed_time;
  }

  virtual ~hexagontask_return_layer(){};

protected:

  explicit hexagontask_return_layer(state_snapshot initial_state, group* g, ::symphony::internal::legacy::task_attrs a)
    : task(initial_state, g, a)
    , _lock_buffers(true)
    , _exec_time(0)
  {}

  explicit hexagontask_return_layer(group* g, ::symphony::internal::legacy::task_attrs a)
    : task(g, a) {
  }

  std::string describe_return_value() {
    return demangler::demangle<void>();
  };

  void unsafe_enable_non_locking_buffer_acquire() {
    _lock_buffers = false;
  }

  template<typename Fn, typename PreacquiredArenas>
  void execute_and_store_retval(Fn& fn, const void *, PreacquiredArenas const*) {

    int status = fn();

    if (status != 0) {
      throw symphony::hexagon_exception();
    }
  }

  template<typename Fn, typename PreacquiredArenas, typename ...TaskArgs>
  void execute_and_store_retval(Fn& fn, const void *requestor, PreacquiredArenas const* p_preacquired_arenas, TaskArgs&&...args) {

    SYMPHONY_INTERNAL_ASSERT(requestor != nullptr, "The requestor should be not null");

    using tuple_args_type = std::tuple<TaskArgs...>;

    using args_buffers_acquire_set_type = buffer_acquire_set< num_buffer_ptrs_in_tuple<tuple_args_type>::value>;

    tuple_args_type args_in_tuple(args...);

    static_assert(std::tuple_size<std::tuple<TaskArgs...>>::value > 0, "Argument tuple should contain at least 1 element");

    args_buffers_acquire_set_type bas;
    if(!_lock_buffers)
      bas.enable_non_locking_buffer_acquire();

    parse_and_add_buffers_to_acquire_set<args_buffers_acquire_set_type,
                                         tuple_args_type,
                                         0,
                                         Fn,
                                         0,
                                         false> parsed_params(bas, args_in_tuple);

    bas.blocking_acquire_buffers(requestor,
                                 {symphony::internal::executor_device::hexagon},
                                 p_preacquired_arenas);

    translate_TP_args_to_hexagon<Fn,
                                 0,
                                 tuple_args_type,
                                 0,
                                 args_buffers_acquire_set_type,
                                 0 == sizeof...(TaskArgs)> hexagon_args_container(args_in_tuple, bas);

    auto & hexagon_args = hexagon_args_container._htp_till_index;

    int status = apply(fn, hexagon_args);

    bas.release_buffers(requestor);

    if (status != 0) {
      throw symphony::hexagon_exception();
    }
  }

private:

  bool _lock_buffers;

  uint64_t _exec_time;

};

template<typename ...TaskArgs>
class hexagontask_arg_layer : public hexagontask_return_layer {

  using parent = hexagontask_return_layer;

  using arity = std::integral_constant<std::uint32_t, sizeof...(TaskArgs)>;

  using storage_arg_types = std::tuple<typename std::remove_cv<typename std::remove_reference<TaskArgs>::type>::type...>;

  template<std::uint32_t ... numbers>
  struct integer_sequence {

    static constexpr size_t get_size() {
      return sizeof...(numbers);
    }
  };

  template<std::uint32_t Top, std::uint32_t ...Sequence>
  struct create_integer_sequence : create_integer_sequence<Top-1, Top-1, Sequence...> {
  };

  template<std::uint32_t ...Sequence>
  struct create_integer_sequence<0, Sequence...> {
    using type = integer_sequence<Sequence...>;
  };

  template<typename HexagonKernel, uint32_t ...Number>
  void expand_args(HexagonKernel& hk, const void* requestor, integer_sequence<Number...>) {

    parent::template execute_and_store_retval(hk.get_fn(),
                                              requestor,
                                              _preacquired_arenas.has_any() ? &_preacquired_arenas : nullptr,
                                              std::move(std::get<Number>(_args))...);

  }

  storage_arg_types _args;

  preacquired_arenas<true, num_buffer_ptrs_in_tuple<storage_arg_types>::value> _preacquired_arenas;

protected:

  std::string describe_arguments() {
    std::string description = "(";
    description += demangler::demangle<TaskArgs...>();
    description +=")";
    return description;
  };

  using state_snapshot = typename parent::state_snapshot;

  hexagontask_arg_layer(state_snapshot initial_state, group* g,
    ::symphony::internal::legacy::task_attrs a, TaskArgs&& ...args)
    : parent(initial_state, g, a)
      , _args(std::forward<TaskArgs>(args)...)
      , _preacquired_arenas()
  {
  }

  template<typename HexagonKernel>
  void prepare_args(HexagonKernel& hk, void const* requestor) {
    expand_args<HexagonKernel>(hk, requestor, typename create_integer_sequence<arity::value>::type());
  }

public:

  virtual ~hexagontask_arg_layer() {

  };

  void unsafe_register_preacquired_arena(bufferstate* bufstate,
                                         arena* preacquired_arena)
  {
    _preacquired_arenas.register_preacquired_arena(bufstate, preacquired_arena);
  }

};

template<typename HexagonKernel, typename ...TaskArgs>
class hexagontask : public hexagontask_arg_layer<TaskArgs...> {

  HexagonKernel _kernel;
  using parent = hexagontask_arg_layer<TaskArgs...>;
  using grandfather = hexagontask_return_layer;

protected:

  using state_snapshot = typename parent::state_snapshot;

public:

  hexagontask(state_snapshot initial_state, group* g, ::symphony::internal::legacy::task_attrs a,
      HexagonKernel&& kernel, TaskArgs&& ...args)
    : parent(initial_state, g, a, std::forward<TaskArgs>(args)...)
    , _kernel(kernel) {
  }

  virtual ~hexagontask() {};

  virtual std::string describe_body() {
    return strprintf("%s%s %" SYMPHONY_FMT_TID, parent::describe_return_value().c_str(),
                     parent::describe_arguments().c_str(),
                     do_get_source());
  };

  uint64_t get_exec_time() const {
    return grandfather::get_exec_time();
  }

  void set_exec_time(uint64_t elapsed_time) {
    grandfather::set_exec_time(elapsed_time);
  }

private:

  virtual uintptr_t do_get_source() const {
    return reinterpret_cast<uintptr_t>(const_cast<HexagonKernel&>(_kernel).get_fn());
  }

  virtual bool do_execute(task_bundle_dispatch* tbd) {
    SYMPHONY_UNUSED(tbd);
    parent::prepare_args(_kernel, this);

    auto start_time = this->get_exec_time();
    if (start_time != 0) {
      uint64_t elapsed_time = symphony_get_time_now() - start_time;
      this->set_exec_time(elapsed_time);
    }

    return true;
  }

  SYMPHONY_DELETE_METHOD(hexagontask(hexagontask const&));
  SYMPHONY_DELETE_METHOD(hexagontask(hexagontask&&));
  SYMPHONY_DELETE_METHOD(hexagontask& operator=(hexagontask const&));
  SYMPHONY_DELETE_METHOD(hexagontask& operator=(hexagontask&&));
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};

#endif
