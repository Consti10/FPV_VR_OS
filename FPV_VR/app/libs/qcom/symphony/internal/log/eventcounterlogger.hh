// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <atomic>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/log/loggerbase.hh>
#include <symphony/internal/log/objectid.hh>

namespace symphony {
namespace internal {
namespace log {

class event_counter_logger : public logger_base {

public:

#ifndef SYMPHONY_USE_EVENT_COUNTER_LOGGER
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
  static void log(Event&& e, event_context&) {
    count(std::forward<Event>(e));
  }

private:

  static void count(events::task_created&&) {
    s_num_tasks.fetch_add(1, symphony::mem_order_relaxed);
  }

  static void count(events::runtime_enabled&& e) {
      s_num_exec_ctx = e.get_num_exec_ctx();
      s_num_tasks_executed = new size_t[s_num_exec_ctx];
      std::fill(s_num_tasks_executed, s_num_tasks_executed + s_num_exec_ctx, 0);
  }

  static void count(events::task_queue_list_created&& e) {
    s_num_qs = e.get_num_queues();
    s_foreign_queue_id = e.get_foreign_queue();
    s_main_queue_id = e.get_main_queue();
    s_device_queues_id = e.get_device_queues();
    s_num_tasks_stolen = new std::atomic<size_t>[s_num_qs];
    std::fill(s_num_tasks_stolen, s_num_tasks_stolen + s_num_qs, 0);
  }

  static void count(events::task_executes&& e) {
    if (e.is_blocking()) {
      s_num_blocking_tasks_exec.fetch_add(1, symphony::mem_order_relaxed);
    } else if (e.is_gpu()) {
      s_num_gpu_tasks_exec.fetch_add(1, symphony::mem_order_relaxed);
    } else if (e.is_inline()) {

      s_num_inline_tasks_exec.fetch_add(1, symphony::mem_order_relaxed);
    } else {

      auto id = e.get_tid();
      s_num_tasks_executed[id]++;
    }
  }

  static void count(events::task_stolen&& e) {

    auto qid = e.get_tq_from();
    s_num_tasks_stolen[qid].fetch_add(1, symphony::mem_order_relaxed);
  }

  static void count(events::group_created&&) {
    s_num_groups.fetch_add(1, symphony::mem_order_relaxed);
  }

  template<typename UnknownEvent>
  static void count(UnknownEvent&&) { }

  static std::atomic<size_t> s_num_tasks;
  static std::atomic<size_t> s_num_groups;
  static size_t s_num_exec_ctx;
  static size_t s_num_qs;
  static size_t s_foreign_queue_id;
  static size_t s_main_queue_id;
  static size_t s_device_queues_id;
  static size_t* s_num_tasks_executed;

  static std::atomic<size_t> s_num_gpu_tasks_exec;

  static std::atomic<size_t> s_num_blocking_tasks_exec;

  static std::atomic<size_t> s_num_inline_tasks_exec;
  static std::atomic<size_t>* s_num_tasks_stolen;

  SYMPHONY_DELETE_METHOD(event_counter_logger());
  SYMPHONY_DELETE_METHOD(event_counter_logger(event_counter_logger const&));
  SYMPHONY_DELETE_METHOD(event_counter_logger(event_counter_logger&&));
  SYMPHONY_DELETE_METHOD(event_counter_logger&
                     operator=(event_counter_logger const&));
  SYMPHONY_DELETE_METHOD(event_counter_logger& operator=(event_counter_logger&&));

};

};
};
};
