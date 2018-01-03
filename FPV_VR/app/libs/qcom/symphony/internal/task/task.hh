// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <algorithm>
#include <memory>

#include <symphony/internal/legacy/attr.hh>
#include <symphony/internal/log/log.hh>
#include <symphony/internal/storage/taskstorage.hh>
#include <symphony/internal/task/exception_state.hh>
#include <symphony/internal/task/task_misc.hh>

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
#include <symphony/internal/memalloc/taskallocator.hh>
#endif

namespace symphony {
namespace internal {

class scheduler;
class task_bundle_dispatch;
class arena;
class bufferstate;
class executor_construct;

template <typename Object> void explicit_unref(Object* obj);
template <typename Object> void explicit_ref(Object* obj);

class task {

public:

  using finish_after_state = task_internal::finish_after_state;
  using finish_after_state_ptr = std::unique_ptr<finish_after_state>;
  using self_ptr = void*;
  using size_type = std::size_t;
  typedef task_internal::state_snapshot state_snapshot;
  typedef task_internal::successor_list successor_list;

private:

  typedef task_internal::lock_guard lock_guard;
  typedef task_internal::state state;
  using exception_state_ptr = symphony::internal::exception_state::exception_state_ptr;
  using blocking_code_container_base = symphony::internal::task_internal::blocking_code_container_base;

  successor_list _successors;
  state _state;
  std::atomic<scheduler*> _scheduler;
  storage_map _taskls_map;

  std::atomic<group*> _current_group;
  self_ptr _self;
  legacy::task_attrs _attrs;

  finish_after_state_ptr _finish_after_state;
  exception_state_ptr _exception_state;
  const log::task_id _log_id;

  blocking_code_container_base* _blocking_code_state;

  std::unique_ptr<std::vector<task*>> _alternatives;

public:

  void execute(task_bundle_dispatch* tbd = nullptr, size_t variant = 0);

  void execute(scheduler& sched, size_t variant = 0) {
    set_scheduler(sched);
    execute_sync(nullptr, variant);
  }

  inline void execute_sync(task_bundle_dispatch* tbd = nullptr, size_t variant = 0);

  inline void execute_inline_task(group* g);

  bool launch(group* g, scheduler* sched) {
    return launch_legacy(g, sched, true);
  }

  bool launch_legacy(group* g, scheduler* sched, bool increase_ref_count);

  bool set_launched(bool increase_ref_count);

  virtual void unsafe_enable_non_locking_buffer_acquire() {
    SYMPHONY_UNREACHABLE("Expect to be called only for tasks that involve buffers");
  }

  virtual void unsafe_register_preacquired_arena(bufferstate* bufstate,
                                                 arena* preacquired_arena)
  {
    SYMPHONY_UNUSED(bufstate);
    SYMPHONY_UNUSED(preacquired_arena);
    SYMPHONY_UNREACHABLE("Expect to be called only for tasks that involve buffers");
  }

  legacy::task_attrs get_attrs() const {
    return _attrs;
  }

  bool is_anonymous() const {
    return has_attr(_attrs, internal::attr::anonymous);
  }

  bool is_blocking() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::blocking);
  }

  bool is_big() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::big);
  }

  bool is_little() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::little);
  }

  bool is_long_running() const {
    return has_attr(_attrs, symphony::internal::attr::long_running);
  }

  bool is_stub() const {
    return has_attr(_attrs, internal::attr::stub);
  }

  bool is_trigger() const {
    return has_attr(_attrs, internal::attr::trigger);
  }

  bool is_pfor() const {
    return has_attr(_attrs, internal::attr::pfor);
  }

  bool is_cancelable() const {
    return !has_attr(_attrs, internal::attr::non_cancelable);
  }

  bool is_yield() const {
    return has_attr(_attrs, internal::attr::yield);
  }

  bool is_gpu() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::gpu);
  }

  bool is_cpu() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::cpu);
  }

  bool
  is_hexagon() const {
    return has_attr(_attrs, symphony::internal::legacy::attr::hexagon);
  }

  bool is_inlined() const {
    return has_attr(_attrs, internal::attr::inlined);
  }

  bool is_bound() {
    return _state.get_snapshot().is_bound();
  }

  bool is_ready(state_snapshot const& snapshot) {
    return snapshot.is_ready(is_cancelable());
  }

  bool is_launched() {
    return get_snapshot().is_launched();
  }

  inline void execute_trigger_task(bool g_was_canceled);

  void execute_blocking_code(blocking_code_container_base* bc);

  bool request_ownership(scheduler& sched) {
    scheduler* expected = nullptr;
    return std::atomic_compare_exchange_strong_explicit(
                                    &_scheduler,
                                    &expected,
                                    &sched,
                                    symphony::mem_order_acq_rel,
                                    symphony::mem_order_relaxed);
  }

  void relinquish_scheduler() {
    _scheduler.store(nullptr, symphony::mem_order_release);
  }

  uintptr_t get_source() const {
    return do_get_source();
  }

  state_snapshot get_snapshot(symphony::mem_order order =
                                symphony::mem_order_acquire) const {
    return _state.get_snapshot(order);
  }

  static constexpr state_snapshot get_initial_state_anonymous() {
    return state_snapshot(task_internal::state::initial_states::anonymous);
  }

  static constexpr state_snapshot get_initial_state_bound() {
    return state_snapshot(task_internal::state::initial_states::unlaunched);
  }

  static constexpr state_snapshot get_initial_state_unbound() {
    return state_snapshot(task_internal::state::initial_states::unbound);
  }

  static constexpr state_snapshot get_initial_state_value_task() {
    return state_snapshot(task_internal::state::initial_states::completed_with_value);
  }

  void predecessor_finished(scheduler* sched, task_bundle_dispatch* tbd = nullptr);

  scheduler* get_scheduler(symphony::mem_order order) {
    return _scheduler.load(order);
  }

  void wait();

  void yield();

  bool cancel() {
    auto l = get_lock_guard();
    return cancel_unlocked();
  }

  bool predecessor_canceled();

  inline bool is_canceled(bool has_cancel_request = false);

  inline void abort_on_cancel();

  void join_group(group* g, bool add_utcache);

  group* get_current_group(symphony::mem_order mem_order =
                           symphony::mem_order_acquire) const {
    return _current_group.load(mem_order);
  }

  inline void join_group_and_set_in_utcache(group* g);

  void reset_in_utcache() {
    _state.reset_in_utcache();
  }

  void add_control_dependency(task* succ);

  bool add_dynamic_control_dependency(task* succ);

  std::string to_string();

  const log::task_id get_log_id() const {
    return _log_id;
  }

  storage_map& get_taskls() {
    return _taskls_map;
  }

  void finish_after(task* t);

  void finish_after(group* g);

  template <typename StubTaskFn>
  void finish_after(task* t, StubTaskFn&& fn) {

    finish_after_impl(t, this, [this, fn] {
      fn();
      finish_after_stub_task_body(this);
    });
  }

  void add_alternative(task* t) {
    if (_alternatives == nullptr) {
      _alternatives = std::unique_ptr<std::vector<task*>>(new std::vector<task*>());
    }
    _alternatives->push_back(t);
  }

  std::vector<task*>& get_alternatives() {
    SYMPHONY_INTERNAL_ASSERT(is_poly(), "must call get_alternatives only on a poly task");
    return *_alternatives;
  }

  task* get_alternative(size_t variant) {
    SYMPHONY_INTERNAL_ASSERT(variant > 0, "variants start from 1");
    SYMPHONY_INTERNAL_ASSERT(_alternatives->size() >= variant,
        "specified variant does not exist");
    auto t = get_alternatives()[variant - 1];
    SYMPHONY_INTERNAL_ASSERT(t != nullptr,
        "specified alternative task does not exist");
    return t;
  }

  bool is_poly() const {
    return (_alternatives != nullptr) && (_alternatives->size() > 0);
  }

  inline size_t get_suitable_alternative(::symphony::internal::task_domain td);

  inline bool is_poly_user_facing(size_t variant) const {
    return variant == 0;
  }

#ifdef SYMPHONY_HAVE_GPU

  void launched_gpu(scheduler* sched, task_bundle_dispatch* tbd) {

    state_snapshot snapshot = _state.lock();
    _successors.gpu_predecessor_finished(sched, tbd);
    snapshot = unlock();
    log::fire_event<log::events::task_launched_into_gpu>(this);
  }
#endif

  void propagate_exception_to(task* t) {
    if (_exception_state == nullptr)
      return;

    auto l = t->get_lock_guard();
    if (t->_exception_state == nullptr) {
      t->_exception_state = exception_state_ptr(new exception_state());
    }
    t->_exception_state->propagate(_exception_state);
  }

  inline void leave_groups_unlocked(bool must_cancel = false);

public:

  virtual ~task();

SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing")

  template<typename Facade>
  Facade* get_facade() {
    static_assert(sizeof(Facade) == sizeof(void*), "Can't allocate Facade.");
    return reinterpret_cast<Facade*>(&_self);
  }

SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing")

  static size_type get_state_offset();

protected:

  task(state_snapshot initial_state, group* g, legacy::task_attrs a):
    _successors(),
    _state(initial_state),
    _scheduler(nullptr),
    _taskls_map(),
    _current_group(g),
    _self(this),
    _attrs(a),
    _finish_after_state(nullptr),
    _exception_state(nullptr),
    _log_id(),
    _blocking_code_state(nullptr),
    _alternatives(nullptr) {

    log::fire_event<log::events::task_created>(this);
  }

  task(group* g, legacy::task_attrs a):
    _successors(),
    _state(has_attr(a, attr::anonymous) ? get_initial_state_anonymous() :
                                          get_initial_state_bound()),
    _scheduler(nullptr),
    _taskls_map(),
    _current_group(g),
    _self(this),
    _attrs(a),
    _finish_after_state(nullptr),
    _exception_state(nullptr),
    _log_id(),
    _blocking_code_state(nullptr),
    _alternatives(nullptr) {

    log::fire_event<log::events::task_created>(this);
  }

  void finish(bool must_cancel,
              std::exception_ptr eptr,
              bool should_set_exception = true,
              bool from_foreign_thread = false);

  void set_bound() {

    _state.set_bound(this);
  }

  successor_list& get_successors() {
    return _successors;
  }

  virtual void propagate_return_value_to_successors(successor_list& succs,
                                                    void* value) {
    succs.propagate_return_value<void>(value);
  }

  void propagate_exceptions_to_successors(successor_list& succs) {
    succs.propagate_exception(this);
  }

  static bool prepare_to_add_task_dependence(task* pred,
                                             task* succ,
                                             bool& should_copy_data);

  static void cleanup_after_adding_task_dependence(task* pred, task* succ);

  static void add_task_dependence(task* pred, task* succ);

  lock_guard get_lock_guard() {
    return lock_guard(*this);
  }

private:

  bool cancel_unlocked();

  void propagate_cancellation_unlocked();

  inline void leave_groups_unlocked(group* g, bool must_cancel = false);

  void set_current_group_unlocked(group* g) {

    _current_group.store(g, symphony::mem_order_relaxed);
  }

  bool may_bypass_scheduler() const {
    return is_gpu() || is_blocking() || is_inlined() || is_hexagon() || is_poly();
  }

  void notify_successors(bool must_cancel,
                         scheduler* sched,
                         successor_list& successors) {
    successors.predecessor_finished(must_cancel, sched);
  }

  void set_scheduler(scheduler& sched) {

    _scheduler.store(&sched, symphony::mem_order_relaxed);
  }

  group* move_current_group_unlocked(symphony::mem_order mem_order =
                                     symphony::mem_order_acquire) {
    auto g = get_current_group(mem_order);
    set_current_group_unlocked(nullptr);
    SYMPHONY_INTERNAL_ASSERT(get_current_group() == nullptr,
        "Unexpected non-null group pointer");
    return g;
  }

  successor_list&& move_successors_unlocked() {
    return std::move(_successors);
  }

  bool is_task_executable_in_domain(::symphony::internal::task_domain td);

  virtual uintptr_t do_get_source() const {
    return 0;
  }

  bool is_exceptional() {
    return _exception_state != nullptr;
  }

  void throw_exceptions() {
    SYMPHONY_INTERNAL_ASSERT(is_exceptional(), "must be called only if exceptional");
    auto guard = get_lock_guard();
    _exception_state->rethrow();
    SYMPHONY_UNREACHABLE("exception should have been thrown");
  }

  void set_exception_unlocked(std::exception_ptr& eptr) {
    SYMPHONY_INTERNAL_ASSERT(eptr != nullptr, "cannot set empty exception");
    if (_exception_state == nullptr) {
      _exception_state = exception_state_ptr(new exception_state());
    }
    _exception_state->add(eptr);
  }

  void propagate_exception_to(group* g);

  virtual std::string describe_body();

  finish_after_state_ptr& get_finish_after_state() {
    return _finish_after_state;
  }

  template <typename StubTask>
  void set_finish_after_state(finish_after_state_ptr& tfa, StubTask const& s) {
    SYMPHONY_INTERNAL_ASSERT((tfa == nullptr),
        "function must be invoked exactly once for each task");
    tfa = finish_after_state_ptr(new finish_after_state());
    tfa->_finish_after_stub_task = s;
  }

  bool should_finish_after(finish_after_state_ptr& tfa) {
    if (tfa == nullptr)
      return false;
    return tfa.get()->_finish_after_stub_task != nullptr;
  }

  template <typename StubTaskFn>
  void finish_after_impl(task* pred, task* succ, StubTaskFn&& fn);

  bool check_and_handle_exceptions(task* t, bool t_canceled, bool propagate = false);

  void launch_finish_after_task(finish_after_state_ptr& fa, bool must_cancel, scheduler* sched);

  void cancel_blocking_code_unlocked();

  virtual bool do_execute(task_bundle_dispatch* tbd = nullptr) = 0;

  inline void finish_after_stub_task_body(task* t);

  virtual void destroy_body_and_args() {};

  void finish_orphan_task(bool in_utcache);

  void finish_poly_task(task* skip_alternative);

  ref_counter_type use_count() const {
    return _state.get_snapshot().get_num_references();
  }

  inline void unref();

  void ref() {
    auto snapshot = _state.ref();
    auto num_refs = snapshot.get_num_references() + 1;
    log::fire_event<log::events::task_reffed>(this, num_refs);
  }

  state_snapshot lock() {
    return _state.lock();
  }

  state_snapshot unlock() {
    return _state.unlock(this);
  }

  bool is_locked() {
    return get_snapshot().is_locked();
  }

  void predecessor_added(bool dynamic_dep = false) {
    _state.add_predecessor(this, dynamic_dep);
  }

  bool set_running();

  void reset_running() {
    _state.reset_running();
  }

  virtual void* get_value_ptr() {
    return nullptr;
  }

  friend struct task_release_manager;
  friend class testing::task_tests;
  friend class task_internal::lock_guard;

  friend class symphony_shared_ptr<task>;
  friend void explicit_unref<task>(task* obj);
  friend void explicit_ref<task>(task* obj);
#ifdef SYMPHONY_HAVE_OPENCL
  friend void CL_CALLBACK completion_callback(cl_event event, cl_int, void* user_data);
  friend void CL_CALLBACK task_bundle_completion_callback(cl_event event, cl_int, void* user_data);
#endif
#ifdef SYMPHONY_HAVE_GLES
  template<typename GLKernel, typename Range, typename GLFence>
  friend
  void launch_gl_kernel(executor_construct const& exec_cons,
                        GLKernel* gl_kernel,
                        Range& global_range,
                        Range& local_range,
                        GLFence& gl_fence);
#endif

  SYMPHONY_DELETE_METHOD(task(task const&));
  SYMPHONY_DELETE_METHOD(task(task&&));
  SYMPHONY_DELETE_METHOD(task& operator=(task const&));
  SYMPHONY_DELETE_METHOD(task& operator=(task&&));
};

inline void
task::unref() {

  auto snapshot = _state.unref();

  log::fire_event<log::events::task_unreffed>(this, snapshot.get_num_references()-1);

  if (snapshot.get_num_references() > 1)
    return;

  SYMPHONY_INTERNAL_ASSERT(snapshot.is_ready_running_or_finished(is_cancelable()) ||
      !snapshot.is_launched(),
      "Unexpected task state."
      "Snapshot: %s\n"
      "Actual: %s\n",
      snapshot.to_string().c_str(),
      to_string().c_str());

  if(snapshot.is_finished()) {
    task_internal::release_manager::release(this);
    return;
  }

  finish_orphan_task(snapshot.in_utcache());
}

inline void
task::execute_sync(task_bundle_dispatch* tbd, size_t variant)
{

  std::atomic_thread_fence(symphony::mem_order_acquire);
  execute(tbd, variant);
}

inline void
task::execute_trigger_task(bool g_was_canceled) {
  SYMPHONY_INTERNAL_ASSERT(is_trigger(), "Task %p is not a trigger task: %s",
                       this,
                       to_string().c_str());

  SYMPHONY_INTERNAL_ASSERT(!get_snapshot().is_launched(),
                       "Can't set trigger task %p to COMPLETED because it's"
                       "already launched: %s",
                       this,
                       to_string().c_str());

  SYMPHONY_INTERNAL_ASSERT(!get_snapshot().is_finished(),
                       "Can't set trigger task %p to COMPLETED because it's"
                       "already launched: %s",
                       this,
                       to_string().c_str());

  SYMPHONY_INTERNAL_ASSERT(!is_cancelable(),
                       "Trigger task %p cannot be cancelable: %s",
                       this,
                       to_string().c_str());

  log::fire_event<log::events::trigger_task_scheduled>(this);
  finish(g_was_canceled, nullptr);
}

inline void
task::add_control_dependency(task* succ)
{
  SYMPHONY_INTERNAL_ASSERT(succ != nullptr, "null task_ptr succ.");

  SYMPHONY_API_THROW(!succ->get_snapshot().is_launched(),
                 "Successor task %p has already been launched: %s",
                 succ,
                 succ->to_string().c_str());

  add_task_dependence(this, succ);

  SYMPHONY_INTERNAL_ASSERT(succ->get_snapshot().is_launched() == false,
                       "Successor task %p was launched while "
                       "setting up dependences: %s",
                       succ,
                       succ->to_string().c_str());
}

inline bool
task::add_dynamic_control_dependency(task* succ)
{
  SYMPHONY_INTERNAL_ASSERT(succ != nullptr, "null task_ptr succ.");
  SYMPHONY_INTERNAL_ASSERT(succ->get_snapshot().is_launched(),
      "Successor task %p should already be launched: %s",
      succ,
      succ->to_string().c_str());
  SYMPHONY_INTERNAL_ASSERT(succ->get_snapshot().is_ready_running_or_finished(succ->is_cancelable()),
      "Successor task %p should already be ready, running, or finished: %s",
      succ,
      succ->to_string().c_str());
  SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_launched(),
      "Predecessor task %p should already be launched: %s",
      this,
      to_string().c_str());
  SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_ready_running_or_finished(succ->is_cancelable()),
      "Predecessor task %p should already be ready, running, or finished: %s",
      this,
      to_string().c_str());

  auto snapshot = lock();

  if ((snapshot.has_cancel_request() && snapshot.is_not_running_yet())
      || snapshot.is_canceled()
      || snapshot.is_completed()) {
    unlock();
    return false;
  }

  _successors.add_control_dependency(succ);
  succ->predecessor_added(true);
  succ->relinquish_scheduler();

  log::fire_event<log::events::task_dynamic_dep>(this, succ);

  unlock();
  return true;
}

inline task*
set_current_task(task* t)
{
  auto ti = tls::get();
  return ti->set_current_task(t);
}

inline task*
current_task() {
  auto ti = tls::get();
  return ti->current_task();
}

inline scheduler*
current_scheduler() {

  if (auto t = current_task())
    return t->get_scheduler(symphony::mem_order_relaxed);
  return nullptr;
}

inline std::tuple<task*, scheduler*>
current_task_and_scheduler() {

  if (auto t = current_task())
    return std::make_tuple(t, t->get_scheduler(symphony::mem_order_relaxed));
  return std::make_tuple<task*, scheduler*>(nullptr, nullptr);
}

inline bool
task::is_task_executable_in_domain(::symphony::internal::task_domain td)
{

  if (is_big()) {
    return (td == task_domain::cpu_big);
  } else if (is_little()) {
    return (td == task_domain::cpu_little);
  } else if (is_cpu()) {
    return (td == task_domain::cpu_all
        || (td == task_domain::cpu_big)
        || (td == task_domain::cpu_little));
  } else if (is_gpu()) {
    return (td == task_domain::gpu);
  } else if (is_hexagon()) {
    return (td == task_domain::hexagon);
  } else {
    return false;
  }
}

size_t
task::get_suitable_alternative(::symphony::internal::task_domain td) {
  SYMPHONY_INTERNAL_ASSERT(is_poly(), "can be called only for a poly task");

  if (is_task_executable_in_domain(td)) {
    return 0;
  }

  auto alternatives = get_alternatives();
  auto alt = std::find_if(alternatives.begin(), alternatives.end(),
      [=](task* t) {
        return t->is_task_executable_in_domain(td);
      });

  SYMPHONY_INTERNAL_ASSERT(alt != alternatives.end(),
      "must have found a suitable alternative");
  return std::distance(alternatives.begin(), alt) + 1;
}

inline
void
task::finish_after_stub_task_body(task* t)
{
  SYMPHONY_INTERNAL_ASSERT(t != nullptr,
      "cannot execute a finish_after stub task for a null task");
  task* st;
  scheduler* sched;
  std::tie(st, sched) = current_task_and_scheduler();
  SYMPHONY_INTERNAL_ASSERT(sched != nullptr,
      "finish_after stub tasks must be executed by a valid scheduler");

  t->set_scheduler(*sched);
  auto snapshot = st->get_snapshot();
  st->propagate_exception_to(t);
  t->finish(snapshot.has_cancel_request(), nullptr);
}

inline void
yield()
{
  if (auto task = current_task()) {
    auto t_ptr = c_ptr(task);
    SYMPHONY_INTERNAL_ASSERT(t_ptr != nullptr, "null task_ptr");
    t_ptr->yield();
  }
}

inline void
task::execute_inline_task(group* g = nullptr)
{
  state_snapshot snapshot;

  SYMPHONY_INTERNAL_ASSERT(is_gpu() == false,
                       "Inline execution of GPU tasks is not supported");

  _state.set_launched(snapshot, true);
  SYMPHONY_INTERNAL_ASSERT(snapshot.is_launched(),
                       "Cannot execute task %p inline "
                       "because it has been launched already: %s.",
                       this, to_string().c_str());

  if (g != nullptr)
    join_group(g, false);

  auto sched = current_scheduler();
  if (sched != nullptr)
    execute(*sched);
  else
    execute_sync();
}

namespace task_internal {

inline
lock_guard::lock_guard(task& task) :
  _task(task),
  _snapshot(_task.lock()) {
}

inline
lock_guard::lock_guard(task& task, state_snapshot snapshot) :
  _task(task),
  _snapshot(snapshot) {

}

inline
lock_guard::~lock_guard()
{
  _task.unlock();
}

inline void
release_manager::release(task* t) {
  SYMPHONY_INTERNAL_ASSERT(debug_is_ok_to_release(t) == true, "Cannot release task t");
#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
  if (!t->is_gpu() && !t->is_hexagon()) {
    t->~task();
    task_allocator::deallocate(t);
  }
  else {
    delete(t);
  }
  return;
#else
  delete(t);
  return;
#endif
}

inline void
successor_list::predecessor_finished(bool canceled, scheduler* sched)
{
  if (is_empty())
    return;

  if (canceled) {
    auto notify_cancel_to_successor = [sched] (task* succ) {
      succ->predecessor_canceled();
      succ->predecessor_finished(sched);
    };

    apply_for_each_and_clear(notify_cancel_to_successor);
  } else {
    auto notify_complete_to_successor = [sched] (task* succ) {
      succ->predecessor_finished(sched);
    };
    apply_for_each_and_clear(notify_complete_to_successor);
  }
}

inline void
successor_list::propagate_exception(task* t)
{
  for_each_successor([t](task* succ){
    t->propagate_exception_to(succ);
  });
}

};
};

};
