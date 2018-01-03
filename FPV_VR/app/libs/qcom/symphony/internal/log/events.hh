// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <thread>

#include <symphony/internal/log/eventsbase.hh>
#include <symphony/internal/storage/schedulerstorage.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {
namespace internal {
namespace log {
namespace events {

struct null_event : public base_event<null_event> {
  static const event_id s_id = event_id_list::null_event;
};

struct unknown_event : public base_event<unknown_event> {
  static const event_id s_id = event_id_list::unknown_event;
};

struct user_log_event_base {
  virtual std::string to_string() = 0;
  virtual ~user_log_event_base() {}
  static const event_id s_id = event_id_list::user_log_event_base;

  static constexpr event_id get_id() { return s_id; }
};

template <typename UserType>
struct user_log_event : public user_log_event_base {
  typedef std::function <std::string (UserType* p)>  U_Func;
  UserType param;
  U_Func func;

  explicit user_log_event (UserType p) : param(p){}

  std::string to_string () {
    return func(&param);
  }
};

struct user_string_event : public base_event<user_string_event> {

  user_string_event() :
    base_event<user_string_event>() {}

  static const char* get_name() {return "user_string_event";}

  static const event_id  s_id = event_id_list::user_string_event;
};

struct buffer_acquire_initiated : public base_event<buffer_acquire_initiated> {

  static const char* get_name() {return "buffer_acquire_initiated";}

  static const event_id  s_id = event_id_list::buffer_acquire_initiated;
};

struct buffer_release_initiated : public base_event<buffer_release_initiated> {

  static const char* get_name() {return "buffer_release_initiated";}

  static const event_id  s_id = event_id_list::buffer_release_initiated;
};

struct buffer_set_acquired : public base_event<buffer_set_acquired> {

  static const char* get_name() {return "buffer_set_acquired";}

  static const event_id  s_id = event_id_list::buffer_set_acquired;
};

struct group_canceled : public single_group_event<group_canceled> {

  group_canceled(group* g, bool success) :
    single_group_event<group_canceled>(g),
    _success(success) {}

  bool get_success() const { return _success; }

  static const char* get_name() {return "group_canceled";}

  static const event_id  s_id = event_id_list::group_canceled;

private:
  const bool _success;
};

struct group_created : public single_group_event<group_created> {

  explicit group_created(group* g) :
    single_group_event<group_created>(g) {}

  static const char* get_name() {return "group_created";}

  static const event_id  s_id = event_id_list::group_created;
};

struct group_destroyed : public single_group_event<group_destroyed> {

  explicit group_destroyed(group* g) :
    single_group_event<group_destroyed>(g) {}

  static const char* get_name() {return "group_destroyed";}

  static const event_id  s_id = event_id_list::group_destroyed;
};

struct group_reffed : public group_ref_count_event<group_reffed> {

  group_reffed(group* g, size_t count) :
    group_ref_count_event<group_reffed>(g, count) {}

  static const char* get_name() {return "group_reffed";}

  static const event_id  s_id = event_id_list::group_reffed;
};

struct group_unreffed : public group_ref_count_event<group_unreffed> {

  group_unreffed(group* g, size_t count) :
    group_ref_count_event<group_unreffed>(g, count) {}

  static const char* get_name() {return "group_unreffed";}

  static const event_id  s_id = event_id_list::group_unreffed;
};

struct group_wait_for_ended : public single_group_event<group_wait_for_ended> {

  explicit group_wait_for_ended(group* g) :
    single_group_event<group_wait_for_ended>(g) {}

  static const char* get_name() {return "group_wait_for_ended";}

  static const event_id  s_id = event_id_list::group_wait_for_ended;
};

struct join_ut_cache : public single_task_event<join_ut_cache> {

  explicit join_ut_cache(task* g) :
    single_task_event<join_ut_cache>(g) {}

    static const char* get_name() {return "join_ut_cache";}

  static const event_id  s_id = event_id_list::join_ut_cache;
};

struct object_reffed : public object_ref_count_event<object_reffed> {

  object_reffed(void* o, size_t count) :
    object_ref_count_event<object_reffed>(o, count) {}

  static const char* get_name() {return "object_reffed";}

  static const event_id  s_id = event_id_list::object_reffed;
};

struct object_unreffed : public object_ref_count_event<object_unreffed> {

  object_unreffed(void* o, size_t count) :
    object_ref_count_event<object_unreffed>(o, count) {}

  static const char* get_name() {return "object_unreffed";}

  static const event_id  s_id = event_id_list::object_unreffed;
};

struct runtime_disabled : public base_event<runtime_disabled> {

  static const char* get_name() {return "runtime_disabled";}

  static const event_id  s_id = event_id_list::runtime_disabled;
};

struct runtime_enabled : public base_event<runtime_enabled> {

  explicit runtime_enabled(size_t num_exec_ctx)
  :_num_exec_ctx(num_exec_ctx) {}

  size_t get_num_exec_ctx() const { return _num_exec_ctx; }

  static const char* get_name() {return "runtime_enabled";}

  static const event_id  s_id = event_id_list::runtime_enabled;

private:
  const size_t _num_exec_ctx;
};

struct scheduler_bundled_task : public single_task_event<scheduler_bundled_task> {

  explicit scheduler_bundled_task(task* t):
    single_task_event<scheduler_bundled_task>(t) {}

  static const char* get_name() {return "scheduler_bundled_task";}

  static const event_id  s_id = event_id_list::scheduler_bundled_task;
};

struct task_after : public dual_task_event<task_after> {

  task_after(task* pred, task* succ) :
    dual_task_event<task_after>(pred, succ) {}

  task* get_pred() const { return get_task(); }

  task* get_succ() const { return get_other_task(); }

  static const char* get_name() {return "task_after";}

  static const event_id  s_id = event_id_list::task_after;
};

struct task_cleanup : public single_task_event<task_cleanup> {

  task_cleanup(task *t, bool canceled, bool in_utcache) :
    single_task_event<task_cleanup>(t),
    _canceled(canceled),
    _in_utcache(in_utcache) {}

  bool get_canceled() const { return _canceled; }

  bool get_in_utcache() const { return _in_utcache; }

    static const char* get_name() {return "task_cleanup";}

    static const event_id  s_id = event_id_list::task_cleanup;

private:
  const bool _canceled;
  const bool _in_utcache;
};

struct task_gpu_completion_callback_invoked : public single_task_event<task_gpu_completion_callback_invoked> {

  explicit task_gpu_completion_callback_invoked(task* t) :
    single_task_event<task_gpu_completion_callback_invoked>(t) {}

    static const char* get_name() {return "task_gpu_completion_callback_invoked";}

  static const event_id  s_id = event_id_list::task_gpu_completion_callback_invoked;
};

struct task_created : public single_task_event<task_created> {

  explicit task_created(task* g) :
    single_task_event<task_created>(g) {}

    static const char* get_name() {return "task_created";}

  static const event_id  s_id = event_id_list::task_created;
};

struct task_destroyed : public single_task_event<task_destroyed> {

  explicit task_destroyed(task* t) :
    single_task_event<task_destroyed>(t) {}

  static const char* get_name() {return "task_destroyed";}

  static const event_id  s_id = event_id_list::task_destroyed;
};

struct task_dynamic_dep : public dual_task_event<task_dynamic_dep> {

  task_dynamic_dep(task* pred, task* succ) :
    dual_task_event<task_dynamic_dep>(pred, succ) {}

  task* get_pred() const { return get_task(); }

  task* get_succ() const { return get_other_task(); }

  static const char* get_name() {return "task_dynamic_dep";}

  static const event_id  s_id = event_id_list::task_dynamic_dep;
};

struct task_executes : public single_task_event<task_executes> {

  task_executes(task* t, bool gpu, bool blocking, bool inlined,
      std::function<device_thread_id(void)> gettid) :
    single_task_event<task_executes>(t), _is_gpu(gpu), _is_blocking(blocking),
    _is_inline(inlined) {
    if (!gpu && !blocking && !inlined) {
      _tid =  gettid();
    }
  }

  static const char* get_name() {return "task_executes";}

  device_thread_id get_tid() {return _tid;}

  bool is_blocking() {return _is_blocking;}

  bool is_gpu() {return _is_gpu;}

  bool is_inline() {return _is_inline;}

  static const event_id  s_id = event_id_list::task_executes;

private:
  bool _is_gpu;
  bool _is_blocking;
  bool _is_inline;
  device_thread_id _tid;
};

struct task_finished : public single_task_event<task_finished> {

  explicit task_finished(task* t) :
    single_task_event<task_finished>(t) {}

  static const char* get_name() {return "task_finished";}

  static const event_id  s_id = event_id_list::task_finished;
};

#ifdef SYMPHONY_HAVE_GPU

struct task_launched_into_gpu : public single_task_event<task_launched_into_gpu> {

  explicit task_launched_into_gpu(task* t):
    single_task_event<task_launched_into_gpu>(t) {}

  static const char* get_name() {return "task_launched_into_gpu";}

  static const event_id  s_id = event_id_list::task_launched_into_gpu;
};
#endif

struct task_sent_to_runtime : public single_task_event<task_sent_to_runtime> {

  explicit task_sent_to_runtime(task* t) :
    single_task_event<task_sent_to_runtime>(t){}

  static const char* get_name() {return "task_sent_to_runtime";}

  static const event_id  s_id = event_id_list::task_sent_to_runtime;
};

struct task_stolen : public single_task_event<task_stolen> {

  task_stolen(task* t, size_t from) :
    single_task_event<task_stolen>(t), _from(from) {}

  static const char* get_name() {return "task_stolen";}

  size_t get_tq_from() {return _from;}

  static const event_id  s_id = event_id_list::task_stolen;

private:
  size_t _from;
};

struct task_queue_list_created : public base_event<task_queue_list_created> {

  task_queue_list_created(size_t num_qs, size_t foreignq, size_t mainq, size_t deviceqs):
    base_event<task_queue_list_created>(),
    _num_qs(num_qs),
    _foreignq(foreignq),
    _mainq(mainq),
    _deviceqs(deviceqs) { }

  static const char* get_name() {return "task_queue_list_created";}

  size_t get_num_queues() {return _num_qs;}

  size_t get_foreign_queue() {return _foreignq;}

  size_t get_main_queue() {return _mainq;}

  size_t get_device_queues() {return _deviceqs;}

  static const event_id  s_id = event_id_list::task_queue_list_created;

private:
  size_t _num_qs, _foreignq, _mainq, _deviceqs;
};

struct task_reffed : public task_ref_count_event<task_reffed> {

  task_reffed(task* t, size_t count) :
    task_ref_count_event<task_reffed>(t, count) {}

  static const char* get_name() {return "task_reffed";}

  static const event_id  s_id = event_id_list::task_reffed;
};

struct task_unreffed : public task_ref_count_event<task_unreffed> {

  task_unreffed(task* t, size_t count) :
    task_ref_count_event<task_unreffed>(t, count) {}

  static const char* get_name() {return "task_unreffed";}

  static const event_id  s_id = event_id_list::task_unreffed;
};

struct task_wait : public single_task_event<task_wait> {

  task_wait(task* t, bool wait_required):
    single_task_event<task_wait>(t),
    _wait_required(wait_required) {

  }

  bool get_wait_required() const {
    return _wait_required;
  }

  static const char* get_name() {return "task_wait";}

  static const event_id  s_id = event_id_list::task_wait;

private:
  const bool _wait_required;
};

struct task_wait_inlined : public single_task_event<task_wait_inlined> {

  explicit task_wait_inlined(task* t):
    single_task_event<task_wait_inlined>(t) {}

  static const char* get_name() {return "task_wait_inlined";}

  static const event_id  s_id = event_id_list::task_wait_inlined;
};

struct trigger_task_scheduled : public single_task_event<trigger_task_scheduled> {

  explicit trigger_task_scheduled(task* t) :
    single_task_event<trigger_task_scheduled>(t) {}

  static const char* get_name() {return "trigger_task_scheduled";}

  static const event_id  s_id = event_id_list::trigger_task_scheduled;
};

struct ws_tree_new_slab : public base_event<ws_tree_new_slab> {

  ws_tree_new_slab() :
    base_event<ws_tree_new_slab>(){}

  static const char* get_name() {return "ws_tree_new_slab";}

  static const event_id  s_id = event_id_list::ws_tree_new_slab;
};

struct ws_tree_node_created : public base_event<ws_tree_node_created> {

  ws_tree_node_created() :
    base_event<ws_tree_node_created>(){}

  static const char* get_name() {return "ws_tree_node_created";}

  static const event_id  s_id = event_id_list::ws_tree_node_created;
};

struct ws_tree_worker_try_own : public base_event<ws_tree_worker_try_own> {

  ws_tree_worker_try_own() :
    base_event<ws_tree_worker_try_own>(){}

  static const char* get_name() {return "ws_tree_worker_try_own";}

  static const event_id  s_id = event_id_list::ws_tree_worker_try_own;
};

struct ws_tree_try_own_success : public base_event<ws_tree_try_own_success> {

  ws_tree_try_own_success() :
    base_event<ws_tree_try_own_success>(){}

  static const char* get_name() {return "ws_tree_try_own_success";}

  static const event_id  s_id = event_id_list::ws_tree_worker_try_own_success;
};

struct ws_tree_worker_try_steal : public base_event<ws_tree_worker_try_steal> {

  ws_tree_worker_try_steal() :
    base_event<ws_tree_worker_try_steal>(){}

  static const char* get_name() {return "ws_tree_worker_try_steal";}

  static const event_id  s_id = event_id_list::ws_tree_worker_try_steal;
};

struct ws_tree_try_steal_success : public base_event<ws_tree_try_steal_success> {

  ws_tree_try_steal_success() :
    base_event<ws_tree_try_steal_success>(){}

  static const char* get_name() {return "ws_tree_try_steal_success";}

  static const event_id  s_id = event_id_list::ws_tree_worker_try_steal_success;
};

};
};
};
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");
