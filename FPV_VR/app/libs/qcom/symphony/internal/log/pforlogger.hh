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
#include <symphony/internal/log/objectid.hh>

namespace symphony {
namespace internal {
namespace log {

class pfor_logger : public logger_base {

public:

  static const auto relaxed = symphony::mem_order_relaxed;

#ifndef SYMPHONY_USE_PFOR_LOGGER
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

  static void count(events::ws_tree_new_slab&&) {
    s_num_slabs.fetch_add(1, relaxed);
  }

  static void count(events::ws_tree_node_created&&) {
    s_num_nodes.fetch_add(1, relaxed);
  }

  static void count(events::ws_tree_worker_try_own&&) {
    s_num_own.fetch_add(1, relaxed);
  }

  static void count(events::ws_tree_worker_try_steal&&) {
    s_num_steal.fetch_add(1, relaxed);
  }

  static void count(events::ws_tree_try_own_success&&) {
    s_success_own.fetch_add(1, relaxed);
  }

  static void count(events::ws_tree_try_steal_success&&) {
    s_success_steal.fetch_add(1, relaxed);
  }

  template<typename UnknownEvent>
  static void count(UnknownEvent&&) { }

  static std::atomic<size_t> s_num_nodes;
  static std::atomic<size_t> s_num_own;
  static std::atomic<size_t> s_num_slabs;
  static std::atomic<size_t> s_num_steal;
  static std::atomic<size_t> s_success_own;
  static std::atomic<size_t> s_success_steal;

  SYMPHONY_DELETE_METHOD(pfor_logger());
  SYMPHONY_DELETE_METHOD(pfor_logger(pfor_logger const&));
  SYMPHONY_DELETE_METHOD(pfor_logger(pfor_logger&&));
  SYMPHONY_DELETE_METHOD(pfor_logger& operator=(pfor_logger const&));
  SYMPHONY_DELETE_METHOD(pfor_logger& operator=(pfor_logger&&));

};

};
};
};
