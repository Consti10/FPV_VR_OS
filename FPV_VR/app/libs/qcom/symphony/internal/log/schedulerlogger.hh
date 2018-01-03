// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <atomic>
#include <map>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/log/loggerbase.hh>
#include <symphony/internal/log/fast_buffer.hh>

namespace symphony {
namespace internal {
namespace log {

class schedulerlogger : public logger_base {

  typedef event_context::counter_type index_type;

  static constexpr index_type s_size = fast_buffer::s_size;

  static const logger_id s_myloggerid = logger_id::schedulerlogger;

public:

#ifndef SYMPHONY_USE_SCHEDULER_LOGGER
  typedef std::false_type enabled;
#else
  typedef std::true_type enabled;
#endif

  static void init();

  static void shutdown();

  static void dump();

  static void paused(){};

  static void resumed(){};

  template<typename Event>
  static void log(Event&& e, event_context& context);

  static std::string to_string(void* str);

  static std::string get_thread_name(std::thread::id thread_id);

  static std::string get_name () { return "SCHD_LOGGER"; }

private:

  static fast_buffer* s_default_buf;

  SYMPHONY_MSC_IGNORE_BEGIN(4100);
  static void logging (events::task_stolen&& e, event_context& context)
  {
    auto count = context.get_count();
    auto pos = count % s_size;
    auto& entry = s_default_buf->get_default_buffer()[pos];
    auto eid = e.get_id();

    entry.reset (count, eid, context.get_this_thread_id(), s_myloggerid);
    std::strcpy (reinterpret_cast<char*>(entry.get_buffer()), "Task stolen\t");
  }
  SYMPHONY_MSC_IGNORE_END(4100);
  template<typename UnknownEvent>
  static void logging(UnknownEvent&&, event_context&) { }

  static event_context::counter_type get_first_entry_pos();

  typedef std::map<std::thread::id, std::string> thread_map;
  static thread_map  s_threads;
  static std::thread::id s_main_thread;

  SYMPHONY_DELETE_METHOD(schedulerlogger());
  SYMPHONY_DELETE_METHOD(schedulerlogger(schedulerlogger const&));
  SYMPHONY_DELETE_METHOD(schedulerlogger(schedulerlogger&&));
  SYMPHONY_DELETE_METHOD(schedulerlogger& operator=(schedulerlogger const&));
  SYMPHONY_DELETE_METHOD(schedulerlogger& operator=(schedulerlogger&&));

};

template<typename Event>
inline void
schedulerlogger::log(Event&& event, event_context& context)
{
  logging (std::forward<Event>(event), context);
}

};
};
};
