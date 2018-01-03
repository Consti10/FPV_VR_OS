// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/tls.hh>

namespace symphony {

namespace internal {
class scheduler;
class task;
class task_bundle_dispatch;

void send_to_runtime(task* task, scheduler* sched = nullptr, bool notify = true,
    task_bundle_dispatch* tbd = nullptr);

void send_to_runtime(task* task, size_t num_instances);

void notify_all(size_t nelem);

size_t get_load_on_current_thread();

extern size_t g_cpu_num_exec_ctx;

extern size_t g_cpu_num_big_exec_ctx;

inline size_t num_execution_contexts() {
  return g_cpu_num_exec_ctx;
}

inline size_t num_big_execution_contexts() {
  return g_cpu_num_big_exec_ctx;
}

inline size_t num_little_execution_contexts() {
  SYMPHONY_INTERNAL_ASSERT(g_cpu_num_exec_ctx >= g_cpu_num_big_exec_ctx,
      "there must be at least as many cpu execution contexts as big cpu execution contexts");
  return g_cpu_num_exec_ctx - g_cpu_num_big_exec_ctx;
}

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

extern size_t g_hexagon_num_exec_ctx;

inline size_t
num_hexagon_execution_contexts() {
  return g_hexagon_num_exec_ctx;
}
#endif

};
};
