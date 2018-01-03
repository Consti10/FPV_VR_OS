// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <atomic>
#include <map>

#include <symphony/internal/log/events.hh>
#include <symphony/internal/log/objectid.hh>
#include <symphony/internal/log/loggerbase.hh>

#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {
namespace log {

class buffer_entry{

public:

  static constexpr size_t s_payload_size = 512;

  SYMPHONY_MSC_IGNORE_BEGIN(4351);
  buffer_entry():
    _logger_id(logger_base::logger_id::unknown),
    _event_count(0),
    _event_id(0),
    _thread_id(),
    _buffer() { }
  SYMPHONY_MSC_IGNORE_END(4351);

  void reset(event_context::counter_type c, event_id e, std::thread::id id,
             logger_base::logger_id lid) {
    _event_count = c;
    _event_id = e;
    _thread_id = id;
    _logger_id = lid;
  }

  event_context::counter_type get_event_count() const {
    return _event_count;
  }

  event_id  get_event_id() const { return _event_id; }

  std::thread::id get_thread_id() const { return _thread_id; };

  void* get_buffer() { return _buffer; }

  logger_base::logger_id get_logger_id () const { return _logger_id; }

private:

  logger_base::logger_id _logger_id;

  event_context::counter_type _event_count;

  event_id  _event_id;

  std::thread::id _thread_id;

  char _buffer[s_payload_size];
};

class fast_buffer {

public:

  typedef event_context::counter_type index_type;

#if defined (SYMPHONY_LOGGER_SIZE)
  static constexpr index_type s_size = SYMPHONY_LOGGER_SIZE;
#else
  static constexpr index_type s_size = 16384;
#endif

  typedef std::array<buffer_entry, s_size> log_array;

  static log_array& get_default_buffer() {
    return s_buffer;
  }

  static fast_buffer* get_default_buffer_ptr() {
    return s_default_buffer_ptr;
  }

  static bool is_default_buffer_empty() {
    buffer_entry& first_entry = s_default_buffer_ptr->s_buffer[0];
    return first_entry.get_event_id() == events::null_event::get_id();
  }

  static void init_default_buffer();

  static std::string get_thread_name(std::thread::id thread_id);

  static event_context::counter_type get_default_first_entry_pos()
  {
    buffer_entry& first_entry = s_default_buffer_ptr->s_buffer[0];
    auto lowest_event_count = first_entry.get_event_count();
    index_type lowest_event_count_pos = 0;

    for ( index_type i = 1; i < s_size; i++) {
      auto current_count = s_default_buffer_ptr->s_buffer[i].get_event_count();
      if (current_count == 0)
        break;
      if (current_count < lowest_event_count) {
        lowest_event_count = current_count;
        lowest_event_count_pos = i;
        break;
      }
    }

    return lowest_event_count_pos;
  }

  static void dump_default_buffer ();

  ~fast_buffer ();

private:

  static fast_buffer* s_default_buffer_ptr;

  static log_array s_buffer;

  typedef std::map<std::thread::id, std::string> thread_map;
  static thread_map  s_threads;
  static std::thread::id s_main_thread;

};

};
};
};
