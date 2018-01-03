// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <forward_list>

#include <symphony/internal/log/log.hh>
#include <symphony/internal/memalloc/groupallocator.hh>
#include <symphony/internal/task/exception_state.hh>
#include <symphony/internal/task/group_signature.hh>
#include <symphony/internal/task/group_state.hh>
#include <symphony/internal/task/group_misc.hh>
#include <symphony/internal/task/group_shared_ptr.hh>
#include <symphony/internal/task/lattice.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {

class group_factory;

namespace testing {
class group_tester;
};

class group;

struct group_refcount_logger {

  static void ref(group* g, ref_counter_type count) {
    log::fire_event<log::events::group_reffed>(g, count);
  }

  static void unref(group* g, ref_counter_type count) {
    log::fire_event<log::events::group_unreffed>(g, count);
  }

};

struct group_release_manager {

  static void release(group* g);
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

class group : public internal::ref_counted_object<group,
                                                  group_refcount_logger,
                                                  group_release_manager> {

SYMPHONY_GCC_IGNORE_END("-Weffc++");

public:

  using self_ptr = void *;

private:

  self_ptr _self;
  using exception_state_ptr = symphony::internal::exception_state::exception_state_ptr;

protected:

  group_misc::group_state _state;

  std::atomic<size_t> _tasks;

  task_shared_ptr _trigger_task;

  task* _representative_task;

  exception_state_ptr _exception_state;

  group_misc::lattice_node* _lattice_node;

  const log::group_id _log_id;

  group_misc::lattice_node::parent_list& parents() const {
    return get_lattice_node()->get_parents();
  };

  group_misc::lattice_node::children_list& children() const {
    return get_lattice_node()->get_children();
  };

protected:

  group(group_misc::id::type id,
      group_misc::group_state::raw_type type):
    _self(this),
    _state(type | id),
    _tasks(0),
    _trigger_task(nullptr),
    _representative_task(nullptr),
    _exception_state(nullptr),
    _lattice_node(nullptr),
    _log_id() {

    log::fire_event<log::events::group_created>(this);

    SYMPHONY_INTERNAL_ASSERT(id != group_misc::id_generator::default_meet_id,
                         "Too many alive leaf groups: %zu", id);
  }

  group() :
    _self(this),
    _state(0),
    _tasks(0),
    _trigger_task(nullptr),
    _representative_task(nullptr),
    _exception_state(nullptr),
    _lattice_node(nullptr),
    _log_id() {

    log::fire_event<log::events::group_created>(this);

  }

  explicit group(group_misc::group_state::raw_type type) :
    _self(this),
    _state(type),
    _tasks(0),
    _trigger_task(nullptr),
    _representative_task(nullptr),
    _exception_state(nullptr),
    _lattice_node(nullptr),
    _log_id() {

    log::fire_event<log::events::group_created>(this);

  }

  SYMPHONY_DELETE_METHOD(group(group const&));
  SYMPHONY_DELETE_METHOD(group(group&&));
  SYMPHONY_DELETE_METHOD(group& operator=(group const&));
  SYMPHONY_DELETE_METHOD(group& operator=(group&&));

public:

  inline group_misc::group_state_snapshot get_state() const {
    return _state.get_snapshot();
  };

  const log::group_id get_log_id() const {
    return _log_id;
  }

  virtual std::string const get_name() const {
    return std::string("");
  }

  std::string to_string(std::string const& msg="") const;

  void wait();

  task_shared_ptr ensure_trigger_task_unless_empty(task* succ = nullptr);

  void set_representative_task(task* t) {
    SYMPHONY_INTERNAL_ASSERT(t->get_current_group() != this, "Representative task "
        "cannot be part of the group. Otherwise, we will end up in a vicious "
        "cycle while checking whether the representative/group is canceled.");
    _representative_task = t;
  }

  group_misc::lock_guard get_lock_guard() {
    return group_misc::lock_guard(_state);
  }

  void set_exception(std::exception_ptr& eptr) {
    SYMPHONY_INTERNAL_ASSERT(eptr != nullptr, "cannot set empty exception");
    auto guard = get_lock_guard();
    if (_exception_state == nullptr) {
      _exception_state = exception_state_ptr(new exception_state());
    }
    _exception_state->add(eptr);
  }

  void propagate_exception_from(exception_state_ptr& eptr) {
    SYMPHONY_INTERNAL_ASSERT(eptr != nullptr, "cannot set empty exception");
    auto guard = get_lock_guard();
    if (_exception_state == nullptr) {
      _exception_state = exception_state_ptr(new exception_state());
    }
    _exception_state->propagate(eptr);
  }

  void cancel();

  bool cancel_if_group_member(task* t) {
    SYMPHONY_INTERNAL_ASSERT(t, "invalid task pointer checked for group "
                         "membership in %p", this);

    auto g = t->get_current_group(symphony::mem_order_relaxed);
    if (g && this->is_ancestor_of(g)) {
      t->cancel();
      return true;
    }
    return false;
  }

  bool is_canceled(symphony::mem_order mem_order=symphony::mem_order_acquire) const {
    SYMPHONY_UNUSED(mem_order);
    return get_state().is_canceled()
        || ((_representative_task != nullptr) && _representative_task->is_canceled(true));
  }

  bool is_exceptional() {
    return _exception_state != nullptr;
  }

  void check_and_handle_exceptions() {
    if (is_exceptional()) {
      _exception_state->rethrow();
      SYMPHONY_UNREACHABLE("exception should have been thrown");
    }
  }

  inline void task_canceled() {
    auto eptr = std::make_exception_ptr(symphony::canceled_exception());
    set_exception(eptr);
  }

  inline void task_canceled(std::exception_ptr& eptr) {
    SYMPHONY_INTERNAL_ASSERT(eptr != nullptr, "eptr cannot be nullptr");
    set_exception(eptr);
  }

  group_misc::group_signature& get_signature() const {
    return get_lattice_node()->get_signature();
  }

  size_t get_order() const {
    return get_lattice_node()->get_order();
  }

  bool is_empty(symphony::mem_order mem_order=symphony::mem_order_seq_cst) const {
    auto num_tasks = _tasks.load(mem_order);
    return (num_tasks == 0);
  }

  bool has_lattice_node() const {return (get_lattice_node() != nullptr);};

  group_misc::lattice_node* get_lattice_node() const {
    return _lattice_node;
  };

  inline void inc_task_counter(group_misc::lattice_lock_policy p =
      group_misc::acquire_lattice_lock);

  inline void dec_task_counter(group_misc::lattice_lock_policy p =
      group_misc::acquire_lattice_lock);

public:

  bool is_ancestor_of(group* g) {
    SYMPHONY_INTERNAL_ASSERT(g, "nullptr group");

    if (this == g)
      return true;

    if (!has_lattice_node() || !g->has_lattice_node())
      return false;

    return (g->get_signature().subset(get_signature()));
  }

  bool is_strict_ancestor_of(group* g) {
    SYMPHONY_INTERNAL_ASSERT(g, "nullptr group");
    return ((this != g) && is_ancestor_of(g));
  }

  bool is_leaf() const {
    return get_state().is_leaf();
  }

  bool create_lattice_node_leaf() {
    if (get_lattice_node() != nullptr)
      return false;
    _lattice_node = new group_misc::lattice_node_leaf(get_id());
    return true;
  }

  bool is_pfor() const {
    return get_state().is_pfor();
  }

  virtual ~group() {
    log::fire_event<log::events::group_destroyed>(this);

    SYMPHONY_INTERNAL_ASSERT(is_empty() == true,
        "Non-empty group: %s" ,
        to_string().c_str());

    auto xxx = use_count();
    if (xxx != 0) {
      SYMPHONY_ALOG("%d %s",xxx, to_string().c_str() );
      symphony_breakpoint();
    }

    #ifdef SYMPHONY_CHECK_INTERNAL
      _state.lock();
      SYMPHONY_INTERNAL_ASSERT(!_trigger_task, "trigger task should be nullptr");
      _state.unlock();
    #endif

    if (_lattice_node != nullptr && is_leaf()) {
      delete _lattice_node;
    }
  }

SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing")

  template<typename Facade>
  Facade* get_facade() {
    static_assert(sizeof(Facade) == sizeof(void*), "Can't allocate Facade.");
    return reinterpret_cast<Facade*>(&_self);
  }

SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing")

protected:

  group_misc::id::type get_id() const {
    return _state.get_snapshot().get_id();
  }

private:

  void inc_task_counter_meet(group_misc::lattice_lock_policy p =
      group_misc::acquire_lattice_lock);

  void dec_task_counter_meet(group_misc::lattice_lock_policy p =
      group_misc::acquire_lattice_lock);

  void propagate_cancellation() {
    group_misc::group_state_snapshot state = get_state();
    if (_state.set_cancel(state))
      if (has_lattice_node())
        for(auto &child : children())
          child->propagate_cancellation();
  }

  void migrate_task_from(group* origin) {

    bool propagated_to_origin = false;
    propagate_task_migration(origin, propagated_to_origin);

    if (!propagated_to_origin)
      origin->dec_task_counter(group_misc::do_not_acquire_lattice_lock);
  }

  void propagate_inc_task_counter() {
    SYMPHONY_INTERNAL_ASSERT(!is_leaf(),"Can't propagate task increase in leaf "
                         "group: %s", to_string().c_str());
    SYMPHONY_INTERNAL_ASSERT(!get_lattice_node()->get_parent_sees_tasks(),
                         "Can't propagate task increase "
                         "in group: %s", to_string().c_str());
    get_lattice_node()->set_parent_sees_tasks(true);
    for (auto& parent: parents())
      c_ptr(parent)->
        inc_task_counter(group_misc::do_not_acquire_lattice_lock);
  }

  void propagate_dec_task_counter() {
    SYMPHONY_INTERNAL_ASSERT(get_lattice_node()->get_parent_sees_tasks(),
                         "Can't propagate task decrease "
                         "in group: %s", to_string().c_str());
    get_lattice_node()->set_parent_sees_tasks(false);
    for (auto& parent: parents())
      c_ptr(parent)->
        dec_task_counter(group_misc::do_not_acquire_lattice_lock);
  }

  void propagate_task_migration(group* origin, bool& propgte_to_origin);

  static std::string get_meet_name();

  inline task_shared_ptr reset_trigger_task() {
    _state.lock();

    if (!is_empty()) {
      _state.unlock();
      return nullptr;
    }
    task_shared_ptr t = std::move(_trigger_task);
    SYMPHONY_INTERNAL_ASSERT(!_trigger_task,
                "std::move() did not reset group trigger task to nullptr");
    _state.unlock();
    return t;
  }

  void launch_trigger_task() {
    auto t = reset_trigger_task();

    auto t_ptr = c_ptr(t);
    if (t_ptr != nullptr) {

      t_ptr->execute_trigger_task(is_canceled(symphony::mem_order_relaxed));
    }
  }

public:

  static size_t get_num_leaves() {
    return group_misc::id_generator::get_num_leaves();
  };

  typedef testing::group_tester tester;

  friend class group_misc::lattice;
  friend class group_factory;
  friend struct group_release_manager;
  friend class testing::group_tester;
};

class leaf: public group
{
public:

  leaf(group_misc::id::type id,
      group_misc::group_state::raw_type type):
    group(id, type) {
  };
};

class named_leaf: public leaf
{
public:

  named_leaf(const char* name, group_misc::id::type id):
    leaf(id, group_misc::group_state::leaf_type()),
    _name(name)
  { }

  virtual std::string const get_name() const {
    return _name;
  }
private:
  std::string _name;

};

class meet: public group
{
public:

  meet(group_misc::group_signature& signature,
       size_t order,
       group_misc::meet_db* db) :
    group(),
    _inlined_lattice_node(signature, order, db) {
    _lattice_node = &_inlined_lattice_node;
  }

  meet(group_misc::group_state::raw_type type,
      size_t order,
      group_misc::meet_db* db):
  group(type),
  _inlined_lattice_node(order, db) {
    _lattice_node = &_inlined_lattice_node;
  }
  virtual ~meet();
private:

  group_misc::lattice_node _inlined_lattice_node;
};

class group_factory
{

public:
  typedef allocatorinternal::
      group_allocator<sizeof(named_leaf), sizeof(meet)> allocator_t;

  static group* create_leaf_group(std::string const& name) {
    auto id = group_misc::id_generator::get_leaf_id();
#if !defined(SYMPHONY_NO_GROUP_ALLOCATOR)
    char* buffer = s_allocator.allocate_leaf(id);
    if (buffer != nullptr) {
      return new (buffer) named_leaf(name.c_str(), id);
    } else {
      return new named_leaf(name.c_str(),id);
    }
#else
    return new named_leaf(name.c_str(),id);
#endif
  }

  static group* create_leaf_group(const char* name) {
    auto id = group_misc::id_generator::get_leaf_id();
#if !defined(SYMPHONY_NO_GROUP_ALLOCATOR)
    char* buffer = s_allocator.allocate_leaf(id);
    if (buffer != nullptr) {
      return new (buffer) named_leaf(name, id);
    } else {
      return new named_leaf(name, id);
    }
#else
    return new named_leaf(name, id);
#endif
  }

  static group* create_leaf_group() {
    auto id = group_misc::id_generator::get_leaf_id();
#if !defined(SYMPHONY_NO_GROUP_ALLOCATOR)
    char* buffer = s_allocator.allocate_leaf(id);
    if (buffer != nullptr) {
      return new (buffer) leaf(id, group_misc::group_state::leaf_type());
    } else {
      return new leaf(id, group_misc::group_state::leaf_type());
    }
#else
    return new leaf(id, group_misc::group_state::leaf_type());
#endif

  }

  static group* create_meet_group(group_misc::group_signature& signature,
                                  group_misc::meet_db* db) {
    SYMPHONY_INTERNAL_ASSERT(db != nullptr,
        "DB for a meet group must not be empty");

    size_t order = signature.popcount();
    group* g = new meet(signature, order, db);
    return g;
  }

  static group* create_pfor_group(group* const& parent_group) {

    group* g = nullptr;
    if (parent_group == nullptr) {
      g = new group(group_misc::group_state::leaf_pfor_type());
      return g;
    }
    if (!parent_group->has_lattice_node()) {

      parent_group->create_lattice_node_leaf();
    }
    g = new meet(group_misc::group_state::meet_pfor_type(),
        parent_group->get_order() + 1,
        parent_group->get_lattice_node()->get_meet_db());
    g->parents().push_front(group_shared_ptr(parent_group));

    std::lock_guard<group_misc::lattice::lock_type>
      lattice_lock(group_misc::lattice::get_mutex());
    parent_group->children().push_front(g);
    return g;
  }

  static group_shared_ptr merge_groups(group* a, group* b, group* current_group = nullptr) {
    return group_misc::lattice::create_meet_node(a, b, current_group);
  }

  static size_t get_leaf_pool_size() {
    return allocator_t::get_leaf_pool_size();
  }

private:

  static allocator_t s_allocator;

};

inline void
group::inc_task_counter(group_misc::lattice_lock_policy lock_policy)
{
  if (is_leaf()) {
    if (_tasks.fetch_add(1, symphony::mem_order_seq_cst) != 0) {

      return;
    }
    explicit_ref(this);

    SYMPHONY_INTERNAL_ASSERT(!is_empty(), "Group must not be empty");
    return;
  }
  inc_task_counter_meet(lock_policy);
}

inline void
group::dec_task_counter(group_misc::lattice_lock_policy lock_policy)
{
  if (is_leaf()) {
    if (_tasks.fetch_sub(1, symphony::mem_order_seq_cst) != 1) {

      return;
    }
    launch_trigger_task();
    explicit_unref(this);
    return;
  }
  dec_task_counter_meet(lock_policy);
}

inline void
group_release_manager::release(group* g)
{
  auto id = g->get_id();
  auto has_valid_id = (g->is_pfor() == false) && g->is_leaf();
#if !defined(SYMPHONY_NO_GROUP_ALLOCATOR)
  if (has_valid_id && (id < group_factory::get_leaf_pool_size())) {

    g->~group();
  }
  else {

    delete(g);
  }
#else
  delete(g);
#endif

  if (has_valid_id)
    group_misc::id_generator::release_id(id);
}

inline void
task::leave_groups_unlocked(group* g, bool must_cancel)
{
  if (g == nullptr)
    return;

  if (must_cancel) {

    g->task_canceled();
  }
  g->dec_task_counter();
}

inline void
task::propagate_exception_to(group* g)
{
  if (g == nullptr)
    return;

  g->propagate_exception_from(_exception_state);
}

inline void
task::leave_groups_unlocked(bool must_cancel)
{

  auto g = get_current_group(symphony::mem_order_relaxed);
  if (must_cancel && is_exceptional()) {
    propagate_exception_to(g);
    must_cancel = false;
  }
  leave_groups_unlocked(g, must_cancel);

  set_current_group_unlocked(nullptr);

  SYMPHONY_INTERNAL_ASSERT(get_current_group(symphony::mem_order_relaxed) == nullptr,
                       "Unexpected non-null group pointer");
}

inline bool
task::set_running()
{

  auto g = get_current_group(symphony::mem_order_relaxed);
  if (g && g->is_canceled(symphony::mem_order_relaxed))
    return false;

  return _state.set_running(is_anonymous(), is_cancelable(), this);
}

inline bool
task::is_canceled(bool has_cancel_request)
{
  if(!is_cancelable())
    return false;

  auto l = get_lock_guard();
  auto current_state = l.get_snapshot();

  if (!has_cancel_request && (current_state.is_running() || current_state.is_completed()))
    return false;

  if (current_state.is_canceled())
    return true;

  if (current_state.has_cancel_request())
    return true;

  if (auto g = get_current_group(symphony::mem_order_relaxed))
    return g->is_canceled();

  return false;
}

inline void
task::join_group_and_set_in_utcache(group* g) {
  SYMPHONY_INTERNAL_ASSERT(is_locked(),
                       "Task must be locked: %s",
                       to_string().c_str());
  SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Nullptr");
  SYMPHONY_INTERNAL_ASSERT(get_current_group() == nullptr,
                       "Task %s already belongs to group %p:%s",
                       to_string().c_str(),
                       g,
                       g->to_string().c_str());

  auto prev_state = _state.set_in_utcache(this);
  SYMPHONY_INTERNAL_ASSERT(!prev_state.in_utcache(),
                       "Task already in utcache: %s",
                       to_string().c_str());

  SYMPHONY_UNUSED(prev_state);

  g->inc_task_counter();

  set_current_group_unlocked(g);
}

inline void
task::abort_on_cancel()
{
  if (!is_anonymous()) {

    auto snapshot = get_snapshot(symphony::mem_order_acquire);
    SYMPHONY_INTERNAL_ASSERT(snapshot.is_running(),
                         "Task %p is not running: %s",
                         this, to_string().c_str());

    if (snapshot.has_cancel_request())
      throw symphony::abort_task_exception();

  }

  auto g = get_current_group(symphony::mem_order_relaxed);
  SYMPHONY_INTERNAL_ASSERT(!is_anonymous() || g,
                       "Anonyous tasks must have a group: %s",
                       to_string().c_str());

  if (g && g->is_canceled(symphony::mem_order_relaxed)) {

    std::atomic_thread_fence(symphony::mem_order_acquire);
    throw symphony::abort_task_exception();
  }
}

inline void
task::finish_after_state::cleanup_groups()
{

  for (auto i : _finish_after_groups) {
    explicit_unref(i);
  }

  _finish_after_groups.clear();
}
};
};

#include <symphony/internal/log/coreloggerevents.hh>
