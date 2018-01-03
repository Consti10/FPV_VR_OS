// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/legacy/device.hh>

#include <symphony/internal/legacy/taskfactory.hh>
#include <symphony/internal/task/group.hh>
#include <symphony/internal/task/task.hh>

namespace symphony {

namespace internal {
namespace legacy {

inline void wait_for(task_shared_ptr const& task)
{
  auto t_ptr = ::symphony::internal::c_ptr(task);
  SYMPHONY_API_ASSERT(t_ptr, "null task_shared_ptr");
  t_ptr->wait();
}

inline void cancel(task_shared_ptr const& task)
{
  auto t_ptr = internal::c_ptr(task);
  SYMPHONY_API_ASSERT(t_ptr, "null task_shared_ptr");
  t_ptr->cancel();
}

inline void after(task_shared_ptr const& pred, task_shared_ptr const& succ)
{
  auto pred_ptr = ::symphony::internal::c_ptr(pred);
  auto succ_ptr = ::symphony::internal::c_ptr(succ);

  SYMPHONY_API_ASSERT(pred_ptr != nullptr, "null task_shared_ptr");
  SYMPHONY_API_ASSERT(succ_ptr != nullptr, "null task_shared_ptr");

  pred_ptr->add_control_dependency(succ_ptr);
}

template<typename Body>
inline task_shared_ptr create_task(Body&& body)
{
  typedef ::symphony::internal::legacy::body_wrapper<Body> wrapper;
  internal::task* t =  wrapper::create_task(std::forward<Body>(body));

  return task_shared_ptr(t, task_shared_ptr::ref_policy::no_initial_ref);
}

inline void launch(task_shared_ptr const& task)
{
  ::symphony::internal::legacy::launch_dispatch(task);
}

template<typename Body>
inline void launch(internal::group_shared_ptr const& a_group, Body&& body, bool notify = true)
{
  ::symphony::internal::legacy::launch_dispatch<Body>(a_group, std::forward<Body>(body), notify);
}

};

template<typename F>
static ::symphony::internal::task_shared_ptr create_stub_task(F f, task* pred = nullptr) {
  auto attrs = legacy::create_task_attrs(attr::stub, attr::non_cancelable);
  if (pred == nullptr)
    attrs = ::symphony::internal::legacy::add_attr(attrs, attr::yield);

  auto stub = ::symphony::internal::legacy::create_task(with_attrs(attrs, f));
  if (pred)
    pred->add_control_dependency(c_ptr(stub));
  return stub;
}

template<typename F>
static void launch_stub_task(F f, task* pred = nullptr) {
  auto stub = create_stub_task(f, pred);
  legacy::launch_dispatch(stub);
}

inline task_shared_ptr create_trigger_task() {
  static auto no_body = []{};
  typedef decltype(no_body) no_body_type;
  typedef internal::task_type_info<no_body_type, true> task_type_info;

  auto attrs = legacy::create_task_attrs(attr::trigger, attr::non_cancelable);

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
  char* task_buffer = task_allocator::allocate(sizeof(cputask<task_type_info>));
  auto t = new (task_buffer) cputask<task_type_info>(nullptr, attrs, std::forward<no_body_type>(no_body));
#else
  auto t = new cputask<task_type_info>(nullptr, attrs, std::forward<no_body_type>(no_body));
#endif

  return task_shared_ptr(t, symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

};
};
