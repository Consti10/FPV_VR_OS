// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/memorder.hh>

class TaskStateSuite;

namespace symphony {
namespace internal {

namespace testing {

class task_tests;

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace task_internal {

class state;

class state_base {

protected:

  typedef uint64_t raw_type;
  typedef uint32_t ref_counter_type;

  enum value : raw_type {

    raw_type_nbits = (sizeof(raw_type) * 8),

    ref_cntr_type_nbits = (sizeof(ref_counter_type) * 8),

    ref_cntr_mask = (raw_type(1) << (ref_cntr_type_nbits)) - 1,

    ref_cntr_max = ref_cntr_mask,

    canceled  = raw_type(1) << (raw_type_nbits - 1),

    completed = canceled   >> 1,

    running = completed  >> 1,

    unlaunched = running >> 1,

    canceled_mask = ~canceled,
    completed_mask = ~completed,
    running_mask = ~running,
    unlaunched_mask = ~unlaunched,

    uninitialized = canceled | completed | running | unlaunched,

    ready_anonymous = 1,

    running_anonymous = running | ready_anonymous,

    cancel_req = unlaunched >> 1,

    in_utcache = cancel_req >> 1,

    locked     = in_utcache >> 1,

    unbound    = locked >> 1,

    cancel_req_mask = ~cancel_req,
    in_utcache_mask = ~in_utcache,
    unlocked_mask   = ~locked,
    unbound_mask   = ~unbound,

    ready_mask = ~(locked | in_utcache | ref_cntr_mask),

    ready_mask_non_cancelable = ~(locked       |
                                 in_utcache    |
                                 ref_cntr_mask |
                                 cancel_req),

    pred_cntr_disp = ref_cntr_type_nbits,

    pred_cntr_mask = ~(canceled   |
                       completed  |
                       running    |
                       unlaunched |
                       cancel_req |
                       in_utcache |
                       locked     |
                       unbound    |
                       ref_cntr_mask),

    pred_cntr_max = (pred_cntr_mask) >> pred_cntr_disp,

    one_pred = (raw_type(1) << (ref_cntr_type_nbits)) + 1,

    one_ref = 1
  };

public:

  friend TaskStateSuite;
};

class state_snapshot : public state_base {

  raw_type _snapshot;

  explicit constexpr state_snapshot(raw_type snapshot) :
    _snapshot(snapshot) {
  }

  friend class ::symphony::internal::task_internal::state;
  friend class ::symphony::internal::task;

  void reset(raw_type snapshot) {
    _snapshot = snapshot;
  }

public:

  using size_type = ref_counter_type;

  state_snapshot() : _snapshot(value::uninitialized) { }

  constexpr raw_type get_raw_value() const {
    return _snapshot;
  }

  std::string to_string() const;

  bool is_finished() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");

    return _snapshot >= completed;
  }

  bool is_completed() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");

    return ((_snapshot & completed) !=0);
  }

  bool is_canceled() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");

    return _snapshot >= canceled;
  }

  bool is_running() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");

    return ((_snapshot & running) != 0);
  }

  bool is_runtime_owned() const {
    static constexpr raw_type RUNTIME_OWNED = (ref_cntr_mask   |
                                               pred_cntr_mask  |
                                               unlaunched |
                                               completed  |
                                               canceled);

    return ((_snapshot & RUNTIME_OWNED) == raw_type(1));
  }

  bool is_ready(bool cancelable_task) const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return (((_snapshot & ready_mask) == 0) ||
            (!cancelable_task &&  (_snapshot & ready_mask_non_cancelable) == 0));
  }

  bool is_ready_running_or_finished(bool cancelable_task) const {
    return _snapshot >= running || is_ready(cancelable_task);
  }

  bool is_launched() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return ((_snapshot & unlaunched) == 0);
  }

  bool in_utcache() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return ((_snapshot & state_base::in_utcache) != 0);
  }

  bool is_bound() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return ((_snapshot & state_base::unbound) == 0);
  }

  bool is_not_running_yet() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");

    return _snapshot < running;
  }

  bool is_locked() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return ((_snapshot & locked) == locked);
  }

  bool has_cancel_request() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return((_snapshot & cancel_req) != 0);
  }

  size_type get_num_predecessors() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return static_cast<size_type>((_snapshot & pred_cntr_mask) >> pred_cntr_disp);
  }

  size_type get_num_references() const {
    SYMPHONY_INTERNAL_ASSERT(_snapshot != value::uninitialized,
                         "State hasn't been initialized yet.");
    return static_cast<size_type>(_snapshot & ref_cntr_mask);
  }

};

class state : public state_base {

private:

  std::atomic<raw_type>  _state;

  std::string get_task_description(task*);

  friend class ::TaskStateSuite;
  friend class ::symphony::internal::testing::task_tests;
  friend class ::symphony::internal::task;

  enum initial_states : state_base::raw_type {
    anonymous = 1,
    unlaunched = state_base::unlaunched | 1,
    unbound = state_base::unlaunched | state_base::unbound | 1,
    completed_with_value = state_base::completed | 1
  };

  explicit state(raw_type raw_value) : _state(raw_value) { }

  explicit state(state_snapshot snapshot) : _state(snapshot.get_raw_value()) { }

  state_snapshot set_finished_and_lock_impl(raw_type finished_state,
                                            ref_counter_type extra_refs) {

    SYMPHONY_INTERNAL_ASSERT(finished_state == completed || finished_state == canceled,
                         "Unknown finished task state.");

    state_snapshot snapshot;
    raw_type desired;

    while(true) {

      do {
        snapshot = get_snapshot(symphony::mem_order_relaxed);
      } while(snapshot.is_locked() == true);

      desired = finished_state | locked | (snapshot.get_num_references() + extra_refs);

      raw_type expected = snapshot.get_raw_value();
      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return state_snapshot(expected);
      }
    };

    SYMPHONY_UNREACHABLE("");
  }

  state_snapshot lock_and_optional_ref_impl(ref_counter_type extra_refs) {

    state_snapshot snapshot;
    raw_type desired;

    while(true) {

      do {
        snapshot = get_snapshot(std::memory_order_relaxed);
      } while(snapshot.is_locked() == true);

      auto raw_snapshot = snapshot.get_raw_value() & ~ref_cntr_mask;
      desired = raw_snapshot | locked | (snapshot.get_num_references() + extra_refs);

      raw_type expected = snapshot.get_raw_value();
      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                std::memory_order_acq_rel,
                                                std::memory_order_acquire)) {
        return state_snapshot(expected);
      }
    };

    SYMPHONY_UNREACHABLE("");
  }

  state_snapshot set_finished_and_unref_and_unlock(bool must_cancel, task* t) {
    ref_counter_type ref_count_dec = 1;

    raw_type finished_state = must_cancel? canceled : completed;

    SYMPHONY_INTERNAL_ASSERT(finished_state == completed || finished_state == canceled,
                         "Unknown finished task state.");

    while(true) {
      auto snapshot = get_snapshot(std::memory_order_relaxed);

      SYMPHONY_INTERNAL_ASSERT(snapshot.is_locked(),
          "Can't unlock task %p because it's not locked: %s",
          t,
          get_task_description(t).c_str());

      raw_type expected = snapshot.get_raw_value();

      raw_type desired = (finished_state
          | (snapshot.get_num_references() - ref_count_dec)) & unlocked_mask;

      if(atomic_compare_exchange_weak_explicit(&_state,
            &expected,
            desired,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
        return state_snapshot(expected);
      }
    }
    SYMPHONY_UNREACHABLE("");
    SYMPHONY_UNUSED(t);
  }

public:

  typedef state_base::raw_type raw_type;

  state_snapshot get_snapshot(symphony::mem_order order = symphony::mem_order_acquire) const {
    return state_snapshot(_state.load(order));
  }

  void add_predecessor(task* t, bool dynamic_dep = false) {

    state_snapshot snapshot (get_snapshot(symphony::mem_order_relaxed));

    while(true) {

      if(((dynamic_dep == false) && (snapshot.is_launched() == true))
          || (snapshot.is_finished() == true)) {
        SYMPHONY_FATAL("Can't add predecessors to task %p State: %s",
                    t,
                    get_task_description(t).c_str());
      }

      raw_type expected = snapshot.get_raw_value();
      raw_type desired  = expected + one_pred;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        if(snapshot.get_num_predecessors() == pred_cntr_max) {
          SYMPHONY_FATAL("Task has reached the maximum number of predecessors");
        }
        return;
      }

      snapshot.reset(expected);
    }
    SYMPHONY_UNREACHABLE("");
    SYMPHONY_UNUSED(t);
  }

  void remove_predecessor(state_snapshot& updated_state) {
    updated_state.reset(_state.fetch_sub(one_pred, symphony::mem_order_acq_rel) - one_pred);
  }

  bool set_launched(state_snapshot& snapshot, bool increase_ref_count) {

    ref_counter_type count = increase_ref_count? 1 : 0;
    snapshot = get_snapshot(symphony::mem_order_relaxed);

    while (true) {

      if ((snapshot.is_launched()) || (snapshot.is_bound() == false)) {
        snapshot.reset(get_snapshot(symphony::mem_order_acquire).get_raw_value());
        return false;
      }

      raw_type expected = snapshot.get_raw_value();
      raw_type desired = (expected & unlaunched_mask) + count;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        snapshot.reset(desired);
        return true;
      }
      snapshot.reset(expected);
    }
    SYMPHONY_UNREACHABLE("");
  }

  bool set_running(bool is_anonymous, bool is_cancelable, task *t) {

    SYMPHONY_INTERNAL_ASSERT(get_snapshot().get_num_predecessors() == 0,
                         "Can't transition task %p to running because it has "
                         "predecessors: %s.",
                         t,
                         get_task_description(t).c_str());

    SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_bound() == true,
                         "Can't transition task %p to running because it isn't "
                         "bound: %s.",
                         t,
                         get_task_description(t).c_str());

    SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_launched(),
                         "Can't transition task %p to running because is not "
                         "launched yet: %s.",
                         t,
                         get_task_description(t).c_str());

    if (is_anonymous) {
      SYMPHONY_INTERNAL_ASSERT(get_snapshot().get_raw_value() == 1,
                           "Anonymous task %p has unexpected state: %s ",
                           t,
                           get_task_description(t).c_str());
      _state.store(running_anonymous, symphony::mem_order_relaxed);
      return true;
    }

    state_snapshot snapshot (get_snapshot(symphony::mem_order_relaxed));

    while (true) {

      if (is_cancelable && snapshot.has_cancel_request()) {
        return false;
      }

      raw_type expected = snapshot.get_raw_value() & (in_utcache | locked | ref_cntr_mask | cancel_req);
      raw_type desired = expected | running;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return true;
      }

      snapshot.reset(expected);
    }

    SYMPHONY_UNREACHABLE("");
    SYMPHONY_UNUSED(t);
  }

  bool reset_running() {
    state_snapshot snapshot (get_snapshot(symphony::mem_order_relaxed));
    while (true) {
      raw_type expected = snapshot.get_raw_value();
      raw_type desired = expected & running_mask;

      if (atomic_compare_exchange_weak_explicit(&_state,
          &expected,
          desired,
          symphony::mem_order_acq_rel,
          symphony::mem_order_acquire)) {
        return true;
      }

      snapshot.reset(expected);
    }

    SYMPHONY_UNREACHABLE("");
  }

  state_snapshot set_finished_and_lock(bool must_cancel, bool is_trigger) {
    raw_type finished_state = must_cancel? canceled : completed;
    ref_counter_type ref_count_inc = is_trigger? 1 : 0;
    return set_finished_and_lock_impl(finished_state, ref_count_inc);
  }

  state_snapshot lock_and_optional_ref(bool inc_ref_count) {
    ref_counter_type ref_count_inc = inc_ref_count? 1 : 0;
    return lock_and_optional_ref_impl(ref_count_inc);
  }

  state_snapshot set_in_utcache(task* t) {
    SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_finished() == false,
                         "Can't set utcache bit on task %p "
                         "because it's done: %s",
                         t,
                         get_task_description(t).c_str());

    SYMPHONY_UNUSED(t);
    return state_snapshot(_state.fetch_or(in_utcache, symphony::mem_order_acq_rel));
  }

  void reset_in_utcache() {
    _state.fetch_and(in_utcache_mask, symphony::mem_order_acq_rel);
  }

  void set_bound(task* t) {
    SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_bound() == false,
                         "Task %p is already bound: %s",
                         t,
                         get_task_description(t).c_str());

    SYMPHONY_UNUSED(t);
    _state.fetch_and(unbound_mask, symphony::mem_order_acq_rel);
  }

  bool set_cancel_request(state_snapshot& snapshot) {
    snapshot.reset(_state.fetch_or(cancel_req, symphony::mem_order_acq_rel));
    if (snapshot.has_cancel_request() || snapshot.is_finished())
      return false;

    snapshot.reset(snapshot.get_raw_value() | cancel_req);
    return true;
  }

  state_snapshot lock() {

    state_snapshot snapshot;

    while(true) {

      do{
        snapshot = get_snapshot(symphony::mem_order_relaxed);
      } while(snapshot.is_locked() == true);

      raw_type expected = snapshot.get_raw_value();
      raw_type desired =  expected | locked;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return state_snapshot(expected);
      }
    };

    SYMPHONY_UNREACHABLE("");
  }

  state_snapshot unlock(task* t) {

    state_snapshot snapshot(get_snapshot(symphony::mem_order_relaxed));

    while(true) {

      SYMPHONY_INTERNAL_ASSERT(snapshot.is_locked(),
                           "Can't unlock task %p because it's not locked: %s",
                           t,
                           get_task_description(t).c_str());

      raw_type expected = snapshot.get_raw_value();
      raw_type desired = expected & unlocked_mask;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return state_snapshot(desired);
      }
      snapshot.reset(expected);
    }
     SYMPHONY_UNREACHABLE("");
     SYMPHONY_UNUSED(t);
  }

  state_snapshot ref() {
    auto old_value = _state.fetch_add(one_ref, symphony::mem_order_acq_rel);
    SYMPHONY_INTERNAL_ASSERT(old_value != ref_cntr_max,
                         "Maximum number of references exceeded.");
    return state_snapshot(old_value);
  }

  state_snapshot unref() {
    auto old_value = _state.fetch_sub(one_ref, symphony::mem_order_acq_rel);
    SYMPHONY_INTERNAL_ASSERT(old_value != 0, "Can't unref task with no references.");
    return state_snapshot(old_value);
  }

};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
};
