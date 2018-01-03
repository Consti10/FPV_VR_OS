// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

namespace symphony {
namespace internal {
namespace log {
namespace events {
namespace event_id_list {
enum events : event_id {

  null_event = 0,
  unknown_event = null_event + 1,

  user_log_event_base = unknown_event + 1,
  user_string_event = user_log_event_base + 1,

  buffer_acquire_initiated = user_string_event + 1,
  buffer_release_initiated = buffer_acquire_initiated + 1,
  buffer_set_acquired = buffer_release_initiated + 1,

  group_canceled = buffer_set_acquired + 1,
  group_created = group_canceled + 1,
  group_destroyed = group_created + 1,
  group_reffed = group_destroyed + 1,
  group_unreffed = group_reffed + 1,
  group_wait_for_ended = group_unreffed + 1,

  join_ut_cache = group_wait_for_ended + 1,
  object_reffed = join_ut_cache + 1,
  object_unreffed = object_reffed + 1,
  runtime_disabled = object_unreffed + 1,
  runtime_enabled = runtime_disabled + 1,
  scheduler_bundled_task = runtime_enabled + 1,
  task_after = scheduler_bundled_task + 1,
  task_cleanup = task_after + 1,
  task_gpu_completion_callback_invoked = task_cleanup + 1,
  task_created = task_gpu_completion_callback_invoked + 1,
  task_destroyed = task_created + 1,
  task_dynamic_dep = task_destroyed + 1,
  task_executes = task_dynamic_dep + 1,
  task_finished = task_executes + 1,
  task_launched_into_gpu = task_finished + 1,
  task_queue_list_created = task_finished + 1,
  task_reffed = task_queue_list_created + 1,
  task_sent_to_runtime = task_reffed + 1,
  task_stolen = task_sent_to_runtime + 1,
  task_unreffed = task_stolen + 1,
  task_wait = task_unreffed + 1,
  task_wait_inlined = task_wait + 1,
  trigger_task_scheduled = task_wait_inlined + 1,
  ws_tree_new_slab = trigger_task_scheduled + 1,
  ws_tree_node_created = ws_tree_new_slab + 1,
  ws_tree_worker_try_own = ws_tree_node_created + 1,
  ws_tree_worker_try_own_success = ws_tree_worker_try_own + 1,
  ws_tree_worker_try_steal = ws_tree_worker_try_own_success + 1,
  ws_tree_worker_try_steal_success = ws_tree_worker_try_steal + 1,

  num_events = ws_tree_worker_try_steal_success + 1
};

};
};
};
};
};
