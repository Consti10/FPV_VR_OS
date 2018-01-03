// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>
#include <tuple>
#include <typeinfo>
#include <type_traits>

#include <symphony/buffer.hh>

#include <symphony/internal/buffer/bufferpolicy.hh>
#include <symphony/internal/task/cputaskinternal.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/demangler.hh>
#include <symphony/internal/util/scopeguard.hh>
#include <symphony/internal/util/strprintf.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {

template<typename ...Stuff> class task_ptr;
template<typename ...Stuff> class task;

template<typename BlockingFunction, typename CancelFunction>
void blocking(BlockingFunction&& bf, CancelFunction&& cf)
{
  auto t = symphony::internal::current_task();
  if(t != nullptr) {
    SYMPHONY_API_THROW(t->is_blocking(),
        "symphony::blocking can only be called from within a blocking task or from outside a task");

    symphony::internal::task_internal::blocking_code_container<BlockingFunction, CancelFunction>
      bcc(std::forward<BlockingFunction>(bf), std::forward<CancelFunction>(cf));
    t->execute_blocking_code(&bcc);
  } else {

    bf();
  }
}

namespace internal {

template <typename ReturnType, typename ...ArgTypes> class cputask_arg_layer;

class group;

class task_bundle_dispatch;

template <typename ReturnType>
class cputask_return_layer : public task {

public:

  using container_type = retval_container<ReturnType>;

  using return_type = typename container_type::type;

  virtual ~cputask_return_layer(){};

  return_type const& get_retval() const {
    return _retval_container._retval;
  }

  return_type&& move_retval() {
    return std::move(_retval_container._retval);
  }

  template<std::uint32_t Pos,
           typename SuccReturnType,
           typename ...SuccArgTypes>
  void add_data_dependency(cputask_arg_layer<SuccReturnType(SuccArgTypes...)> *succ) {
    static_assert(sizeof...(SuccArgTypes) >= 1,
        "Invalid task arity.");
    bool should_copy_data = false;
    if (!prepare_to_add_task_dependence(this, succ, should_copy_data)) {
      if (should_copy_data) {
        succ->template set_arg<Pos>(_retval_container._retval);
      }
      return;
    }

    get_successors().add_data_dependency(succ, succ->template get_arg<Pos>(), succ->template should_move<Pos>());

    cleanup_after_adding_task_dependence(this, succ);
  }

  template <typename ReturnT>
  void store_retval(ReturnT&& retval) {
    _retval_container = retval;
  }

protected:

  template<typename ...Args>
  explicit cputask_return_layer(state_snapshot initial_state,
                                group* g,
                                legacy::task_attrs a,
                                Args&& ...args)
    : task(initial_state, g, a),
      _retval_container(return_type(std::forward<Args>(args)...)) {
  }

  template<typename ...Args>
  explicit cputask_return_layer(group* g,
                                legacy::task_attrs a,
                                Args&& ...args)
    : task(g, a),
      _retval_container(return_type (std::forward<Args>(args)...)) {
  }

  std::string describe_return_value() {
    return demangler::demangle<ReturnType>();
  };

  template<typename Body, typename ...Args>
  void execute_and_store_retval(Body& b, Args&&...args) {
    _retval_container = b(args...);
  }

  virtual void propagate_return_value_to_successors(successor_list& successors, void* value) {

    successors.propagate_return_value<return_type>(value);
  }

private:

  virtual void* get_value_ptr() {
    return &_retval_container._retval;
  }

  retval_container<ReturnType> _retval_container;

};

template <>
class cputask_return_layer<void> : public task {

public:

  virtual ~cputask_return_layer(){};

protected:

  cputask_return_layer(state_snapshot initial_state,
                       group* g,
                       legacy::task_attrs a)
    : task(initial_state, g, a) {
  }

  cputask_return_layer(group* g,
                       legacy::task_attrs a)
    : task(g, a) {
  }

  std::string describe_return_value() {
    return "void";
  };

  template<typename Body, typename ...Args>
  void execute_and_store_retval(Body& b, Args&&... args) {
    b(args...);
  }

};

template<typename ReturnType>
class value_cputask : public cputask_return_layer<ReturnType> {

  using state_snapshot = typename cputask_return_layer<ReturnType>::state_snapshot;

public:

  template<typename... Args>
  explicit value_cputask(state_snapshot initial_state,
                         group *g,
                         legacy::task_attrs attrs,
                         Args&& ...args) :
    cputask_return_layer<ReturnType>(initial_state, g, attrs, std::forward<Args>(args)...) {
  }

  using return_type = ReturnType;

  virtual std::string describe_body() {
    return strprintf("value_cputask<%s>", demangler::demangle<ReturnType>().c_str());
  }

  virtual ~value_cputask() {}

private:

  virtual bool do_execute(task_bundle_dispatch* tbd = nullptr) {
    SYMPHONY_UNUSED(tbd);
    SYMPHONY_UNREACHABLE("Completed tasks cannot execute.");
    return false;
  }

  virtual void destroy_body_and_args(){
    SYMPHONY_UNREACHABLE("Cannot destroy the body of a completed task.");
  }

  SYMPHONY_DELETE_METHOD(value_cputask(value_cputask const&));
  SYMPHONY_DELETE_METHOD(value_cputask(value_cputask&&));
  SYMPHONY_DELETE_METHOD(value_cputask& operator=(value_cputask const&));
  SYMPHONY_DELETE_METHOD(value_cputask& operator=(value_cputask&&));

};

template<typename ReturnType>
class cputask_arg_layer<ReturnType()> : public cputask_return_layer<ReturnType> {

  using parent = cputask_return_layer<ReturnType>;
  using parent_return_type = typename std::decay<ReturnType>::type;

  static_assert(std::is_same<ReturnType, void>::value ||
                std::is_default_constructible<parent_return_type>::value,
                "Task return type for a non-value task must be default_constructible.");

public:

  virtual ~cputask_arg_layer(){};

protected:

  using state_snapshot = typename parent::state_snapshot;

  std::string describe_arguments() {
    return "()";
  };

  cputask_arg_layer(state_snapshot initial_state,
                    group* g,
                    legacy::task_attrs a)
    : parent(initial_state, g, a) {
  }

  cputask_arg_layer(group* g,
                    legacy::task_attrs a)
    : parent(g, a) {
  }

  template<typename Body>
  void prepare_args(Body& b) {
    parent::template execute_and_store_retval<Body>(b);
  }

  template<typename Body>
  ::symphony::task_ptr<ReturnType> prepare_args_but_do_not_store_retval(Body& b) {
    return b();
  }

  void destroy_args() {

  }

  void* acquire_buffers() {

    return nullptr;
  }

public:
  void release_buffers(void*) {

  }

};

template<typename ReturnType, typename Arg1, typename ...Rest>
class cputask_arg_layer<ReturnType(Arg1, Rest...)> : public cputask_return_layer<ReturnType> {

  using parent = cputask_return_layer<ReturnType>;

  using arity = std::integral_constant<std::uint32_t, 1 + sizeof...(Rest)>;

  using orig_arg_types = std::tuple<Arg1, Rest...>;

  using storage_arg_types = std::tuple<typename task_arg_type_info<Arg1>::storage_type,
                                       typename task_arg_type_info<Rest>::storage_type...>;

  template<std::uint32_t Pos>
  using orig_type = typename std::tuple_element<Pos, orig_arg_types>::type;

  template<std::uint32_t Pos, typename ArgType>
  void set_arg(ArgType&& value) {
    static_assert(Pos < arity::value, "Out-of-range Pos value");

    using expected_type = orig_type<Pos>;

    using is_rvalue_ref = typename std::is_rvalue_reference<expected_type>::type;

    set_arg_impl<Pos, ArgType>(std::forward<ArgType>(value), is_rvalue_ref());
  }

  template<std::uint32_t Pos, typename ArgType>
  void set_arg_impl(ArgType&& value, std::true_type) {
    static_assert(Pos < arity::value, "Out-of-range Pos value");
    std::get<Pos>(_args) = std::move(value);
  }

  template<std::uint32_t Pos, typename ArgType>
  void set_arg_impl(ArgType&& value, std::false_type) {
    static_assert(Pos < arity::value, "Out-of-range Pos value");
    std::get<Pos>(_args) = value;
  }

  template<std::uint32_t Pos,
           typename PredReturnType>
  void set_data_dependency_impl(cputask_return_layer<PredReturnType>* pred) {
    static_assert(std::is_same<PredReturnType, void>::value == false,
                  "Can't bind a task that returns void.");

    SYMPHONY_INTERNAL_ASSERT( pred != nullptr, "Unexpected null pointer.");

    pred->template add_data_dependency<Pos,
                                       ReturnType,
                                       Arg1,
                                       Rest...>(this);
  }

  template<std::uint32_t Pos, typename ArgType>
  struct bind_by_value {
    static void bind_impl(ArgType&& value, cputask_arg_layer<ReturnType(Arg1, Rest...)>* curr_task) {
      static_assert(is_by_value_t<ArgType&&>::value == true,
                    "Argument has to be bound by symphony::by_value.");

      using user_arg      = typename is_by_value_t<ArgType&&>::type;

      curr_task->set_arg<Pos,user_arg>(std::forward<user_arg>(value._t));
    }
  };

  template<std::uint32_t Pos, typename ArgType>
  struct bind_by_data_dep {
    static void bind_impl(ArgType&& value, cputask_arg_layer<ReturnType(Arg1, Rest...)>* curr_task) {
      static_assert(is_by_data_dep_t<ArgType&&>::value == true,
                    "Argument has to be bound by symphony::by_data_dependency.");
      using user_arg         = typename is_by_data_dep_t<ArgType&&>::type;
      using user_arg_noref   = typename std::remove_reference<user_arg>::type;
      using user_arg_nocvref = typename std::remove_cv<user_arg_noref>::type;

      static_assert(is_symphony_task20_ptr<user_arg_nocvref>::has_return_value == true,
                    "Argument has to be a symphony api20 task_ptr with return type information");
      using pred_return_type = typename is_symphony_task20_ptr<user_arg_nocvref>::type;

      auto pred_ptr =
        static_cast<cputask_return_layer<pred_return_type>*>(::symphony::internal::get_cptr(value._t));
      curr_task->set_data_dependency_impl<Pos, pred_return_type>(pred_ptr);
    }
  };

  template<std::uint32_t Pos, typename ArgType>
  struct bind_as_value {
    static void bind_impl(ArgType&& value, cputask_arg_layer<ReturnType(Arg1, Rest...)>* curr_task) {
      static_assert(is_by_value_t<ArgType&&>::value == false,
                    "Argument has to be bound as value.");
      curr_task->set_arg<Pos,ArgType&&>(std::forward<ArgType>(value));
    }
  };

  template<std::uint32_t Pos, typename ArgType>
  struct bind_as_data_dep {
    static void bind_impl(ArgType&& value, cputask_arg_layer<ReturnType(Arg1, Rest...)>* curr_task) {
      static_assert(is_by_data_dep_t<ArgType&&>::value == false,
                    "Argument has to be bound as data dependency.");

      using user_arg_noref   = typename std::remove_reference<ArgType&&>::type;
      using user_arg_nocvref = typename std::remove_cv<user_arg_noref>::type;

      static_assert(is_symphony_task20_ptr<user_arg_nocvref>::has_return_value == true,
                    "Argument has to be a symphony api20 task_ptr with return type information");
      using pred_return_type = typename is_symphony_task20_ptr<user_arg_nocvref>::type;

      auto pred_ptr = static_cast<cputask_return_layer<pred_return_type>*>(::symphony::internal::get_cptr(value));
      curr_task->set_data_dependency_impl<Pos, pred_return_type>(pred_ptr);
    }
  };

  template<std::uint32_t Pos, typename ArgType>
  void bind(ArgType&& value) {
    SYMPHONY_CONSTEXPR_CONST bool is_by_value    = is_by_value_t<ArgType&&>::value;
    SYMPHONY_CONSTEXPR_CONST bool is_by_data_dep = is_by_data_dep_t<ArgType&&>::value;

    static_assert(!is_by_value || !is_by_data_dep,
                  "Cannot bind by value and by data dependency at the same time.");

    using user_arg = typename std::conditional<is_by_value,
                                               typename is_by_value_t<ArgType&&>::type,
                                               typename is_by_data_dep_t<ArgType&&>::type>::type;
    using expected = typename std::tuple_element<Pos, orig_arg_types>::type;

    SYMPHONY_CONSTEXPR_CONST bool can_as_value    = can_bind_as_value<user_arg, expected>::value;
    SYMPHONY_CONSTEXPR_CONST bool can_as_data_dep = can_bind_as_data_dep<user_arg, expected>::value;

    SYMPHONY_CONSTEXPR_CONST bool by_value    = is_by_value && can_as_value;
    SYMPHONY_CONSTEXPR_CONST bool by_data_dep = is_by_data_dep && can_as_data_dep;
    SYMPHONY_CONSTEXPR_CONST bool as_value    = !is_by_value && can_as_value;
    SYMPHONY_CONSTEXPR_CONST bool as_data_dep = !is_by_data_dep && can_as_data_dep;

    SYMPHONY_CONSTEXPR_CONST bool undecided   =
      (as_value && as_data_dep && !by_value && !by_data_dep) == true;
    SYMPHONY_CONSTEXPR_CONST bool error       =
      (as_value || as_data_dep || by_value || by_data_dep) == false;

    static_assert(undecided == false,
                  "Ambiguous argument binding. Please specifiy by using symphony::by_value() or symphony::by_data_dependency().");
    static_assert(error == false,
                  "Argument binding type mis-match. Cannot bind argument.");

    using bind_policy = typename std::conditional<
                          by_value,
                          bind_by_value<Pos, ArgType>,
                          typename std::conditional<
                            by_data_dep,
                            bind_by_data_dep<Pos, ArgType>,
                            typename std::conditional<
                              as_value,
                              bind_as_value<Pos, ArgType>,
                              bind_as_data_dep<Pos, ArgType>
                            >::type
                          >::type
                        >::type;
    bind_policy::bind_impl(std::forward<ArgType>(value), this);
  }

  template<std::uint32_t ... numbers>
  struct integer_sequence {

    static SYMPHONY_CONSTEXPR_CONST size_t get_size() {
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

  template<typename Body, uint32_t ...Number>
  void expand_args(Body& b, integer_sequence<Number...>) {
    parent::template execute_and_store_retval(b, std::move(std::get<Number>(_args))...);
  }

  template<typename Body, uint32_t ...Number>
  ::symphony::task_ptr<ReturnType> expand_args_but_do_not_store_retval(Body& b, integer_sequence<Number...>) {
    return b(std::move(std::get<Number>(_args))...);
  }

  storage_arg_types _args;
  set_arg_tracker _set_arg_tracker;

  template<std::uint32_t Pos>
  void destroy_args_impl(std::false_type) {
    std::get<Pos>(_args).destroy();
    destroy_args_impl<Pos+1>(typename std::conditional<(Pos + 1) < arity::value,
                             std::false_type, std::true_type>::type());
  }

  template<std::uint32_t Pos>
  void destroy_args_impl(std::true_type) {

  }

  static constexpr size_t num_buffer_args = num_buffer_ptrs_in_tuple<std::tuple<Arg1, Rest...>>::value;
  using buffer_acquire_set_t = buffer_acquire_set< num_buffer_args >;

  template <typename BufferPtr>
  void acquire_buffer(buffer_acquire_set_t& bas, BufferPtr& b, std::true_type, bool do_not_dispatch) {

    if(b == nullptr)
      return;

    if(do_not_dispatch == true) {
      bas.add(b,
              std::conditional< is_const_buffer_ptr<BufferPtr>::value,
                                std::integral_constant<bufferpolicy::action_t, bufferpolicy::acquire_r>,
                                std::integral_constant<bufferpolicy::action_t, bufferpolicy::acquire_rw> >::type::value);
    }
    else {
      auto acquired_arena = bas.find_acquired_arena(b);
      SYMPHONY_INTERNAL_ASSERT(acquired_arena != nullptr, "Error. Acquired arena is nullptr");
      arena_storage_accessor::access_mainmem_arena_for_cputask(acquired_arena);
      reinterpret_cast<symphony::internal::buffer_ptr_base&>(b).allocate_host_accessible_data(true);
    }
  }

  template <typename NotBufferPtr>
  void acquire_buffer(buffer_acquire_set_t&, NotBufferPtr&, std::false_type, bool) {

  }

  template<std::uint32_t Pos>
  void acquire_buffers_impl(buffer_acquire_set_t& bas, std::false_type, bool do_not_dispatch) {
    using type = typename std::tuple_element<Pos, storage_arg_types>::type::type;
    using is_buffer = is_api20_buffer_ptr<type>;

    acquire_buffer(bas, std::get<Pos>(_args).get_larg(), is_buffer(), do_not_dispatch);
    acquire_buffers_impl<Pos+1>(bas,
                                typename std::conditional<(Pos + 1) < arity::value,
                                                          std::false_type,
                                                          std::true_type>::type(),
                                do_not_dispatch);
  }

  template<std::uint32_t Pos>
  void acquire_buffers_impl(buffer_acquire_set_t&, std::true_type, bool) {

  }

  template<size_t Pos, typename Arg1Type, typename ...RestType>
  void bind_all_impl(Arg1Type&& arg1, RestType&&...rest) {
    bind<Pos>(std::forward<Arg1Type>(arg1));
    bind_all_impl<Pos + 1>(std::forward<RestType>(rest)...);
  }

  template<size_t Pos , typename Arg1Type>
  void bind_all_impl(Arg1Type&& arg1) {
    bind<Pos>(std::forward<Arg1Type>(arg1));
  }

  template <std::uint32_t Pos>
  void* get_arg() {
    return &(std::get<Pos>(_args));
  }

  template <std::uint32_t Pos>
  bool should_move() {
    using expected_type = orig_type<Pos>;
    using is_rvalue_ref = typename std::is_rvalue_reference<expected_type>::type;
    return is_rvalue_ref();
  }

  template <typename RT>
  friend class cputask_return_layer;

protected:

  std::string describe_arguments() {
    std::string description = "(";
    description += demangler::demangle<Arg1, Rest...>();
    description +=")";
    return description;
  };

  using state_snapshot = typename parent::state_snapshot;

  cputask_arg_layer(group* g,
                    legacy::task_attrs a)
    : parent(g, a),
      _args(),
      _set_arg_tracker(arity::value),
      _lock_buffers(true),
      _preacquired_arenas()
  {}

  cputask_arg_layer(state_snapshot initial_state,
                    group* g,
                    legacy::task_attrs a)
    : parent(initial_state, g, a),
      _args(),
      _set_arg_tracker(arity::value),
      _lock_buffers(true),
      _preacquired_arenas()
  {}

  template<typename Arg1Type, typename ...RestType>
  cputask_arg_layer(state_snapshot initial_state,
                    group* g,
                    legacy::task_attrs a,
                    Arg1Type&& arg1,
                    RestType&& ...rest)
    : parent(initial_state, g, a),
      _args(),
      _set_arg_tracker(set_arg_tracker::do_not_track()),
      _lock_buffers(true),
      _preacquired_arenas()
  {

    static_assert(sizeof...(RestType) + 1 == arity::value, "Invalid number of arguments");

    bind_all_impl<0, Arg1Type, RestType...>(std::forward<Arg1Type>(arg1), std::forward<RestType>(rest)...);
  }

  template<typename Body>
  void prepare_args(Body& b) {
    SYMPHONY_INTERNAL_ASSERT(_set_arg_tracker.are_all_set() == true, "Arguments are not ready");
    expand_args<Body>(b, typename create_integer_sequence<arity::value>::type());

  }

  template<typename Body>
  ::symphony::task_ptr<ReturnType> prepare_args_but_do_not_store_retval(Body& b) {
    SYMPHONY_INTERNAL_ASSERT(_set_arg_tracker.are_all_set() == true, "Arguments are not ready");
    return expand_args_but_do_not_store_retval<Body>(b, typename create_integer_sequence<arity::value>::type());
  }

  void destroy_args() {
    destroy_args_impl<0>(std::false_type());
  }

  buffer_acquire_set_t acquire_buffers()
  {
    buffer_acquire_set_t bas;
    if(!_lock_buffers)
      bas.enable_non_locking_buffer_acquire();

    acquire_buffers_impl<0>(bas, std::false_type(), true);

    bas.blocking_acquire_buffers(this,
                                 {symphony::internal::executor_device::cpu},
                                 _preacquired_arenas.has_any() ? &_preacquired_arenas : nullptr);

    acquire_buffers_impl<0>(bas, std::false_type(), false);

    return bas;
  }

public:

  void release_buffers(buffer_acquire_set_t& bas) {
    bas.release_buffers(this);
  }

  virtual ~cputask_arg_layer() {

  };

  template<typename Arg1Type, typename ...RestType>
  void bind_all(Arg1Type&& arg1, RestType&&...rest) {
    static_assert(sizeof...(RestType) + 1 == arity::value, "Invalid number of arguments");

    set_arg_tracker::size_type error_pos;
    bool success;

    std::tie(success, error_pos) = _set_arg_tracker.set_all(arity::value);

    SYMPHONY_API_THROW(success, "Argument %d was already set", error_pos);

    bind_all_impl<0, Arg1Type, RestType...>(std::forward<Arg1Type>(arg1), std::forward<RestType>(rest)...);

    task::set_bound();
  }

  void unsafe_enable_non_locking_buffer_acquire() {
    _lock_buffers = false;
  }

  void unsafe_register_preacquired_arena(bufferstate* bufstate,
                                         arena* preacquired_arena)
  {
    _preacquired_arenas.register_preacquired_arena(bufstate, preacquired_arena);
  }

protected:

  bool _lock_buffers;

  preacquired_arenas<true, num_buffer_args> _preacquired_arenas;

};

template <typename TaskTypeInfo>
class cputask : public cputask_arg_layer<typename TaskTypeInfo::final_signature> {

  using task_info = TaskTypeInfo;
  using user_code_container = typename task_info::container;
  user_code_container _user_code_container;

  using parent = cputask_arg_layer<typename task_info::final_signature>;

protected:

  using state_snapshot = typename parent::state_snapshot;

public:

  template<typename UserCode, typename... Args>
  cputask(state_snapshot initial_state, group* g, legacy::task_attrs a, UserCode&& user_code, Args&& ...args)
    : parent(initial_state, g, a, std::forward<Args>(args)...)
    , _user_code_container(std::forward<UserCode>(user_code))
  {}

  template<typename UserCode, typename... Args>
  cputask(group* g, legacy::task_attrs a, UserCode&& user_code, Args&& ...args)
    : parent(g, a, std::forward<Args>(args)...)
    , _user_code_container(std::forward<UserCode>(user_code)) {
  }

  virtual ~cputask() {};

  virtual std::string describe_body() {
    return strprintf("%s%s %s", parent::describe_return_value().c_str(),
                     parent::describe_arguments().c_str(),
                     _user_code_container.to_string().c_str());
  };

private:

  virtual uintptr_t do_get_source() const {
    return _user_code_container.get_source();
  }

  virtual bool do_execute(task_bundle_dispatch* tbd) {
    SYMPHONY_UNUSED(tbd);

    auto bas = parent::acquire_buffers();
    auto release_buffers_scope_guard = make_scope_guard([this, &bas]{
        parent::release_buffers(bas);
      });

    do_execute_dispatch(typename task_info::collapse_actual());

    return true;
  }

  virtual void destroy_body_and_args() {
    _user_code_container.destroy_user_code();
    parent::destroy_args();
  }

  void do_execute_dispatch(std::true_type) {
    auto inner_task = parent::prepare_args_but_do_not_store_retval(_user_code_container);
    this->finish_after(c_ptr(inner_task), [this, inner_task]{
      if (!inner_task->canceled()) {
        parent::template store_retval(inner_task->copy_value());
      }
    });

    inner_task->launch();
  }

  void do_execute_dispatch(std::false_type){
    parent::prepare_args(_user_code_container);
  }

  SYMPHONY_DELETE_METHOD(cputask(cputask const&));
  SYMPHONY_DELETE_METHOD(cputask(cputask&&));
  SYMPHONY_DELETE_METHOD(cputask& operator=(cputask const&));
  SYMPHONY_DELETE_METHOD(cputask& operator=(cputask&&));
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
