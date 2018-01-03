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

class corelogger : public logger_base {

  typedef event_context::counter_type index_type;

  static constexpr index_type s_size = fast_buffer::s_size;

  static const logger_id s_myloggerid = logger_id::corelogger;

public:

#ifndef SYMPHONY_USE_CORELOGGER
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

  static std::string to_string (void* str);

  static std::string get_thread_name(std::thread::id thread_id);

  static std::string get_name () { return "CORE_LOGGER"; }

private:

  static fast_buffer* s_default_buf;

  typedef std::map<std::thread::id, std::string> thread_map;
  static thread_map  s_threads;
  static std::thread::id s_main_thread;

  SYMPHONY_DELETE_METHOD(corelogger());
  SYMPHONY_DELETE_METHOD(corelogger(corelogger const&));
  SYMPHONY_DELETE_METHOD(corelogger(corelogger&&));
  SYMPHONY_DELETE_METHOD(corelogger& operator=(corelogger const&));
  SYMPHONY_DELETE_METHOD(corelogger& operator=(corelogger&&));

};

};
};
};
