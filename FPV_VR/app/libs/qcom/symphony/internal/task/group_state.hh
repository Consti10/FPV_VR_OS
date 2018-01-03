// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <climits>

#include <symphony/internal/util/debug.hh>

class GroupStateSuite;

namespace symphony {
namespace internal {

class group;

namespace group_misc {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

class group_state_base {

protected:

  typedef size_t raw_type;

  static constexpr raw_type raw_type_nbits   = (sizeof(raw_type) * CHAR_BIT);
  static constexpr raw_type canceled   = raw_type(1) << (raw_type_nbits - 1);
  static constexpr raw_type locked  = canceled   >> 1;
  static constexpr raw_type ref_counted  = locked   >> 1;
  static constexpr raw_type leaf_type_flag  = ref_counted   >> 1;
  static constexpr raw_type pfor_type_flag    = leaf_type_flag  >> 1;

  static constexpr raw_type canceled_mask   = ~canceled;
  static constexpr raw_type locked_mask     = ~locked;
  static constexpr raw_type ref_counted_mask     = ~ref_counted;
  static constexpr raw_type leaf_type_flag_mask     = ~leaf_type_flag;
  static constexpr raw_type pfor_type_flag_mask     = ~pfor_type_flag;
  static constexpr raw_type unlocked_mask   = ~locked;

  static constexpr raw_type id_mask = ~(canceled |
                                  locked |
                                  ref_counted |
                                  leaf_type_flag |
                                  pfor_type_flag);

public:

  static const raw_type MAX_ID = id_mask  - 2;

};

class group_state_snapshot : public group_state_base {

  friend class group_state;
  friend class ::GroupStateSuite;

  static const raw_type UNINITIALIZED_STATE = id_mask - 1;

  raw_type _state;

  constexpr  group_state_snapshot(raw_type state) : _state(state) { }

public:

  typedef size_t size_type;

  group_state_snapshot() : _state(UNINITIALIZED_STATE) { }

  constexpr raw_type get_raw_value() const {
    return _state;
  }

  std::string to_string() const {return "";};

  bool is_leaf() const {
    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet.");

    return ((_state & leaf_type_flag) != 0);
  }

  bool is_pfor() const {
    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet.");

    return ((_state & pfor_type_flag) != 0);
  }

  bool is_ref_counted() const {
    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet.");

    return ((_state & ref_counted) == ref_counted);
  }

  bool is_canceled() const {

    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet");

    return ((_state & canceled) == canceled);
  }

  bool is_locked() const {
    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet.");
    return ((_state & locked) == locked);
  }

  size_type get_id() const {
    SYMPHONY_INTERNAL_ASSERT(_state != UNINITIALIZED_STATE,
                         "State hasn't been initialized yet.");
    return static_cast<size_type>(_state & id_mask);
  }

};

class group_state : public group_state_base {

  std::atomic<raw_type>  _state;

  friend class symphony::internal::group;
  friend class ::GroupStateSuite;

  explicit group_state(raw_type state) : _state(state) { }

public:

  typedef group_state_base::raw_type raw_type;

  static raw_type leaf_pfor_type() {return pfor_type_flag | leaf_type_flag;};

  static raw_type meet_pfor_type() {return pfor_type_flag;};

  static raw_type leaf_type() {return leaf_type_flag;};

  group_state_snapshot get_snapshot(symphony::mem_order order =
                                   symphony::mem_order_relaxed) const {
    return group_state_snapshot(_state.load(order));
  }

  bool set_cancel(group_state_snapshot& state){

    if (state.is_canceled())
      return false;
    state = _state.fetch_or(canceled, symphony::mem_order_acq_rel);

    if (state.is_canceled())
      return false;
    state = state.get_raw_value() | canceled;
    return true;
  }

  group_state_snapshot lock() {

    group_state_snapshot prior_state;

    while(true) {

      do{
        prior_state = get_snapshot(symphony::mem_order_relaxed);
      } while(prior_state.is_locked() == true);

      raw_type expected = prior_state.get_raw_value();
      raw_type desired =  expected | locked;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return expected;
      }
    };

    SYMPHONY_UNREACHABLE("lock returns only if successful");
  }

  group_state_snapshot unlock() {

    group_state_snapshot state(get_snapshot(symphony::mem_order_relaxed));

    while(true){

      SYMPHONY_INTERNAL_ASSERT(state.is_locked(),
                           "Can't unlock group because it's not locked");

      raw_type expected = state.get_raw_value();
      raw_type desired = expected & unlocked_mask;

      if (atomic_compare_exchange_weak_explicit(&_state,
                                                &expected,
                                                desired,
                                                symphony::mem_order_acq_rel,
                                                symphony::mem_order_acquire)) {
        return true;
      }
      state = expected;
    }
     SYMPHONY_UNREACHABLE("lock returns only if successful");
  }

};

class lock_guard {

  group_state& _state;

public:

  inline explicit lock_guard(group_state& state)
  : _state(state) {
    _state.lock();
  }

  inline ~lock_guard() {
    _state.unlock();
  }
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
};
