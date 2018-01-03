// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <thread>

#include <iostream>

#include <symphony/internal/log/corelogger.hh>
#include <symphony/internal/task/group.hh>

namespace symphony {
namespace internal {
namespace log {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

#define SYMPHONY_ADD_TO_MEMBUF_MAP(EVENT)                                   \
  template<>                                                            \
  struct event_handler_map<events::EVENT> {                             \
    typedef EVENT##_handler handler;                                    \
  };                                                                    \
                                                                        \

template <typename EVENT>
struct event_handler_map;

struct event_handler_base {
  typedef seq_object_id<task> task_id;
  typedef seq_object_id<group> group_id;

  virtual std::string to_string() const = 0;
  virtual ~event_handler_base() {}

protected:
  static std::string get_task_name(task_id);
  static std::string get_group_name(group_id);
};

class ref_counted_event_handler : public event_handler_base {

  void* _object;
  const size_t _count;

protected:

  template<typename EVENT>
  explicit ref_counted_event_handler(EVENT &&e) :
  _object(e.get_object()),
  _count(e.get_count()) { }

  void* get_object() const { return _object; }
  size_t get_count() const { return _count; }
};

class single_group_event_handler : public event_handler_base {

  group *_group;
  group_id _group_id;

protected:

  template<typename EVENT>
  explicit single_group_event_handler(EVENT &&e):
  _group(e.get_group()),
  _group_id(e.get_group()->get_log_id()) { }

  group* get_group() const { return _group; }
  std::string get_group_name() const;
};

class single_task_event_handler : public event_handler_base {

  task *_task;
  task_id _task_id;

protected:

  template<typename EVENT>
  explicit single_task_event_handler(EVENT &&e):
  _task(e.get_task()),
  _task_id(e.get_task()->get_log_id()) { }

  task* get_task() const { return _task; }
  std::string get_task_name() const;
};

class dual_task_event_handler : public single_task_event_handler {

  task *_other_task;
  task_id _other_task_id;

protected:

  template<typename EVENT>
  explicit dual_task_event_handler(EVENT &&e) :
    single_task_event_handler(std::forward<EVENT>(e)),
    _other_task(e.get_other_task()),
    _other_task_id(e.get_other_task()->get_log_id()) { }

  task* get_other_task() const { return _other_task; }
  std::string get_other_task_name() const;
};

struct group_canceled_handler : public single_group_event_handler {

  typedef events::group_canceled event;

  explicit group_canceled_handler(event&& e) :
    single_group_event_handler(std::forward<event>(e)),
    _success(e.get_success()) { }

  std::string to_string() const;

private:

  bool _success;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_canceled);

struct group_created_handler : public single_group_event_handler {

  typedef events::group_created event;

  explicit group_created_handler(event&& e):
    single_group_event_handler(std::forward<event>(e)),
    _is_leaf(e.get_group()->is_leaf()) { }

  std::string to_string() const;

private:

  bool _is_leaf;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_created);

struct group_destroyed_handler : public single_group_event_handler {

  typedef events::group_destroyed event;

  explicit group_destroyed_handler(event&& e):
    single_group_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_destroyed);

struct group_reffed_handler : public single_group_event_handler {

  typedef events::group_reffed event;

  explicit group_reffed_handler(event&& e):
    single_group_event_handler(std::forward<event>(e)),
    _count(e.get_count()) { }

  std::string to_string() const;

private:

  size_t _count;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_reffed);

struct group_unreffed_handler : public single_group_event_handler {

  typedef events::group_unreffed event;

  explicit group_unreffed_handler(event&& e):
    single_group_event_handler(std::forward<event>(e)),
    _count(e.get_count()) { }

  std::string to_string() const;

private:

  size_t _count;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_unreffed);

struct group_wait_for_ended_handler : public single_group_event_handler {

  typedef events::group_wait_for_ended event;

  explicit group_wait_for_ended_handler(event&& e):
    single_group_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(group_wait_for_ended);

struct join_ut_cache_handler : public single_task_event_handler{

  typedef events::join_ut_cache event;

  explicit join_ut_cache_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(join_ut_cache);

struct object_reffed_handler : public ref_counted_event_handler {

  typedef events::object_reffed event;

  explicit object_reffed_handler(event&& e):
    ref_counted_event_handler(std::forward<event>(e)){ }

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(object_reffed);

struct object_unreffed_handler : public ref_counted_event_handler {

  typedef events::object_unreffed event;

  explicit object_unreffed_handler(event&& e):
    ref_counted_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(object_unreffed);

struct task_after_handler : public dual_task_event_handler {

  typedef events::task_after event;

  explicit task_after_handler(event&& e):
    dual_task_event_handler(std::forward<event>(e)){}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_after);

struct task_cleanup_handler : public single_task_event_handler {

  typedef events::task_cleanup event;

  explicit task_cleanup_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)),
    _canceled(e.get_canceled()),
    _in_utcache(e.get_in_utcache()) { }

  std::string to_string() const;

private:

  bool _canceled;
  bool _in_utcache;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_cleanup);

struct task_created_handler : public single_task_event_handler {

  typedef events::task_created event;

  explicit task_created_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)),
    _attrs(e.get_task()->get_attrs()) { }

  std::string to_string() const;

private:
  ::symphony::internal::legacy::task_attrs _attrs;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_created);

struct task_destroyed_handler : public single_task_event_handler {

  typedef events::task_destroyed event;

  explicit task_destroyed_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_destroyed);

struct task_finished_handler : public single_task_event_handler {

  typedef events::task_finished event;

  explicit task_finished_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_finished);

#ifdef SYMPHONY_HAVE_GPU
struct task_launched_into_gpu_handler : public single_task_event_handler {

  typedef events::task_launched_into_gpu event;

  explicit task_launched_into_gpu_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_launched_into_gpu);
#endif

struct task_executes_handler : public single_task_event_handler {

  typedef events::task_executes event;

  explicit task_executes_handler(event&&e ) :
    single_task_event_handler(std::forward<event>(e)),
    _group(e.get_task()->get_current_group(symphony::mem_order_relaxed)),
    _group_id(e.get_task()->get_current_group(symphony::mem_order_relaxed)?
              e.get_task()->get_current_group(symphony::mem_order_relaxed)->get_log_id(): null_object_id<group>()),
    _anonymous(e.get_task()->is_anonymous()) {
  }

  std::string to_string() const;

  group* get_group() const { return _group; }
  bool is_anonymous() const { return _anonymous; }

private:

  std::string get_group_name() const {
    if (!_group)
      return "no group";
    return event_handler_base::get_group_name(_group_id);
  }

  group* _group;
  group_id _group_id;

  bool _anonymous;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_executes);

struct task_sent_to_runtime_handler : public single_task_event_handler {

  typedef events::task_sent_to_runtime event;

  explicit task_sent_to_runtime_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_sent_to_runtime);

struct task_reffed_handler : public single_task_event_handler {

  typedef events::task_reffed event;

  explicit task_reffed_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)),
    _count(e.get_count()) { }

  std::string to_string() const;

private:

  size_t _count;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_reffed);

struct task_unreffed_handler : public single_task_event_handler {

  typedef events::task_unreffed event;

  explicit task_unreffed_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)),
    _count(e.get_count()) { }

  std::string to_string() const;

private:

  size_t _count;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_unreffed);

struct task_wait_handler : public single_task_event_handler {

  typedef events::task_wait event;

  inline explicit task_wait_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)),
    _wait_required(e.get_wait_required())
  { }
  std::string to_string() const;

  const bool _wait_required;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_wait);

struct task_wait_inlined_handler : public single_task_event_handler {

  typedef events::task_wait_inlined event;

  inline explicit task_wait_inlined_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)){ }
  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(task_wait_inlined);

struct runtime_disabled_handler : public event_handler_base {

  typedef events::runtime_disabled event;

  explicit runtime_disabled_handler(event&&) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(runtime_disabled);

struct runtime_enabled_handler : public event_handler_base {

  typedef events::runtime_enabled event;

  explicit runtime_enabled_handler(event&& e) :
    _num_exec_ctx(e.get_num_exec_ctx()) { }
  std::string to_string() const;

private:

  size_t _num_exec_ctx;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(runtime_enabled);

struct trigger_task_scheduled_handler : public single_task_event_handler {

  typedef events::trigger_task_scheduled event;

  explicit trigger_task_scheduled_handler(event&& e):
    single_task_event_handler(std::forward<event>(e)) {}

  std::string to_string() const;
};
SYMPHONY_ADD_TO_MEMBUF_MAP(trigger_task_scheduled);

struct unknown_event_handler : public event_handler_base {

  typedef events::unknown_event event;

  std::string to_string() const;

  template<typename EVENT>
  explicit unknown_event_handler(EVENT &&e):
  _event_id(e.get_id()),
  _event_name(e.get_name()){ }

private:

  event_id _event_id;
  const char* _event_name;
};

template<typename EVENT>
struct event_handler_map {
  typedef unknown_event_handler handler;

};

template<typename Event>
inline void
corelogger::log(Event&& event, event_context& context)
{

  auto count = context.get_count();
  auto pos =  count % s_size;
  auto& entry = s_default_buf->get_default_buffer()[pos];
  typedef typename event_handler_map<Event>::handler handler;

  static_assert(sizeof(handler) <= buffer_entry::s_payload_size,
                "Handler is larger than entry payload.");

  entry.reset(count, handler::event::get_id(), context.get_this_thread_id(),
              s_myloggerid);

  new (entry.get_buffer()) handler(std::forward<Event>(event));
}

#undef SYMPHONY_ADD_TO_MEMBUF_MAP

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
};
