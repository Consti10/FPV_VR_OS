// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <utility>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/templatemagic.hh>

#include "events.hh"
#include "loggerbase.hh"
#include "fast_buffer.hh"
#include "objectid.hh"

namespace symphony {
namespace internal {
namespace log {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename ...LS> struct any_logger_enabled;
template<>
struct any_logger_enabled<> : public std::integral_constant<bool, false> { };

template<typename L1, typename...LS>
struct any_logger_enabled<L1, LS...> :
    public std::integral_constant<bool, (L1::enabled::value)?
                                  true :
                                  any_logger_enabled<LS...>::value> {
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename ...T> struct method_dispatcher;

template<> struct method_dispatcher<> {
  template<typename Event>
  static void log(Event&&, event_context&){ };
  static void init(){ };
  static void shutdown(){ };
  static void dump(){ };
  static void pause(){ };
  static void resume(){ };
};

template<typename L1, typename...LS>
struct method_dispatcher<L1, LS...>
{
private:

  template<typename Event>
  static void _log(Event&& e, event_context& context, std::true_type) {
    L1::log(std::forward<Event>(e), context);
  }

  template<typename Event>
  static void _log(Event&&, event_context&, std::false_type) {}

public:

  template<typename Event>
  static void log(Event&& e, event_context& context) {
    _log(std::forward<Event>(e), context, typename L1::enabled());
    method_dispatcher<LS...>::log(std::forward<Event>(e), context);
  }

  static void init() {
    static_assert(std::is_base_of<logger_base, L1>::value,
                  "\nAll symphony loggers must inherit from logger_base."
                  "\nPlease read loggerbase.hh to learn how to "
                  "write a logger");

    symphony::internal::log::fast_buffer::init_default_buffer();

    if (L1::enabled::value) {
      L1::init();
    }
    method_dispatcher<LS...>::init();
  }

  static void shutdown() {
    if (L1::enabled::value)
      L1::shutdown();
    method_dispatcher<LS...>::shutdown();
  }

  static void dump() {
    if (L1::enabled::value)
      L1::dump();
    method_dispatcher<LS...>::dump();
  }

  static void pause() {
    if (L1::enabled::value)
      L1::paused();
    method_dispatcher<LS...>::pause();
  }

  static void resume() {
    if (L1::enabled::value)
      L1::resumed();
    method_dispatcher<LS...>::resume();
  }
};

template <typename L1, typename ...LN>
class infrastructure {

private:

  typedef typename any_logger_enabled<L1, LN...>::integral_constant
    any_logger_enabled;

  typedef method_dispatcher<L1, LN...> loggers;

  enum class status : unsigned {
    uninitialized,
    active,
    paused,
    finished
  };
  static status s_status;

  template<typename Event>
  static void _event(Event&&, std::false_type) {}

  template<typename Event>
  static void _event(Event&& e, std::true_type) {
    if (s_status != status::active)
      return;

    event_context context;
    loggers::log(std::forward<Event>(e), context);
  }

public:

  typedef typename std::conditional<any_logger_enabled::value,
                                    seq_object_id<task>,
                                    null_object_id<task>>::type task_id;

  typedef typename std::conditional<any_logger_enabled::value,
                                    seq_object_id<group>,
                                    null_object_id<group>>::type group_id;

  typedef any_logger_enabled enabled;

  static void init() {
    if(any_logger_enabled::value == false ||
       s_status != status::uninitialized)
      return;

    static_assert(duplicated_types<L1, LN...>::value == false,
                  "\nDuplicated logger types in logging infrastructure.");

    loggers::init();

    s_status = status::active;
  }

  static void shutdown() {
    if(any_logger_enabled::value == false ||
       s_status == status::uninitialized ||
       s_status == status::paused)
      return;
    fast_buffer::dump_default_buffer();
    loggers::shutdown();
    s_status = status::paused;
  }

  static void pause() {
    if (any_logger_enabled::value == false ||
        s_status == status::paused)
      return;
    s_status = status::paused;
    loggers::pause();
  }

  static void resume() {
    if(any_logger_enabled::value == false ||
       s_status == status::active)
      return;

    s_status = status::active;
    loggers::resume();
  }

  static bool is_initialized() {
    return (s_status == status::active) || (s_status == status::paused);
  }

  static bool is_active() {
    return (s_status == status::active);
  }

  static bool is_paused() {
    return (s_status == status::paused);
  }

  static void dump() {
    loggers::dump();
  }

  template<typename Event>
  static void event_fired(Event&& e) {
    _event(std::forward<Event>(e), any_logger_enabled());
  }

  SYMPHONY_DELETE_METHOD(infrastructure());
  SYMPHONY_DELETE_METHOD(infrastructure(infrastructure const&));
  SYMPHONY_DELETE_METHOD(infrastructure(infrastructure&&));
  SYMPHONY_DELETE_METHOD(infrastructure& operator=(infrastructure const&));
  SYMPHONY_DELETE_METHOD(infrastructure& operator=(infrastructure&&));
};

template <typename L1, typename ...LN>
typename infrastructure<L1, LN...>::status
  infrastructure<L1, LN...>::s_status =
  infrastructure::status::uninitialized;

};
};
};
