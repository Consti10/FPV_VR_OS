// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>

#include <symphony/internal/compat/compat.h>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/log/common.hh>
#include <symphony/internal/log/eventids.hh>
#include <symphony/internal/log/eventcontext.hh>

namespace symphony {
namespace internal {

class task;
class group;

namespace log {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename Event>
class base_event {

public:

  static constexpr event_id get_id() { return Event::s_id; }

  typedef std::function <void (Event&&, event_context&)>  handler_func;
};

template<typename ObjectType, typename Event>
struct single_object_event : public base_event<Event> {

  explicit single_object_event(ObjectType* object) :
  _object(object) {}

  ObjectType* get_object() const {return _object; }

private:
  ObjectType* const _object;
};

template <typename Event>
struct single_group_event : public single_object_event<group, Event> {

  explicit single_group_event(group* g) :
    single_object_event<group, Event>(g) { }

  group* get_group() const {
    return single_object_event<group, Event>::get_object();
  }

};

template <typename Event>
struct group_ref_count_event : public single_group_event<Event> {

  group_ref_count_event(group* g, size_t count) :
    single_group_event<Event>(g),
    _count(count) {}

  size_t get_count() const { return _count; }

private:
  const size_t _count;
};

template <typename Event>
struct single_task_event : public single_object_event<task, Event> {

  explicit single_task_event(task* g) :
    single_object_event<task, Event>(g) { }

  task* get_task() const {
    return single_object_event<task, Event>::get_object();
  }

};

template <typename Event>
struct task_ref_count_event : public single_task_event<Event> {

  task_ref_count_event(task* t, size_t count) :
    single_task_event<Event>(t),
    _count(count) {}

  size_t get_count() const { return _count; }

private:
  const size_t _count;

};

template <typename Event>
struct dual_task_event : public single_task_event<Event> {

  dual_task_event(task* t1, task* t2) :
    single_task_event<Event>(t1),
    _other(t2) { }

  task* get_other_task() const { return _other;}

private:
  task* const _other;
};

template<typename Event>
struct object_ref_count_event : public base_event<Event> {

  object_ref_count_event(void* o, size_t count) :
    _o(o),
    _count(count)
  {}

  void* get_object() const {return _o; }

  size_t get_count() const {return _count; }

private:
  void* const _o;
  const size_t _count;
};

};
};
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");
