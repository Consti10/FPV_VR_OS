// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <cstdint>

#include <symphony/internal/scheduler/task_domain.hh>
#include <symphony/internal/scheduler/thread_type.hh>

#include <symphony/internal/util/tlsptr.hh>

namespace symphony {
namespace internal {

class task;

namespace tls {

class info {

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)

  static size_t get_next_allocator_id() {
    static std::atomic<size_t> s_allocator_id_counter(0);
    auto id = s_allocator_id_counter.fetch_add(1);
    return id;
  }
#endif

public:

  info() :
    _task(nullptr)
    , _thread_id()
    , _thread_type(internal::thread_type::foreign)
    , _task_domain(internal::task_domain::cpu_all)
#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
    , _allocator_id(get_next_allocator_id())
#endif
    {

    }

  SYMPHONY_DEFAULT_METHOD(~info());
  SYMPHONY_DELETE_METHOD(info(const info &));
  SYMPHONY_DELETE_METHOD(info& operator=(const info&));

  task* current_task() const {
    return _task;
  }

  task* set_current_task(task* t) {
    auto prev = _task;
    _task = t;
    return prev;
  }

  uintptr_t thread_id() const {
    return _thread_id;
  }

  void set_thread_id(uintptr_t tid) {
    _thread_id = tid;
  }

  bool is_main_thread() const {
    return _thread_type == internal::thread_type::main;
  }

  void set_main_thread();

  bool is_foreign_thread() const {
    return _thread_type == internal::thread_type::foreign;
  }

  bool is_symphony_thread() const {
    return _thread_type == internal::thread_type::symphony;
  }

  void set_symphony_thread();

  task_domain get_task_domain() {
    return _task_domain;
  }

  void set_task_domain(task_domain td) {
    _task_domain = td;
  }

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
  size_t allocator_id() const {
    return _allocator_id;
  }
#endif

private:

  internal::task* _task;
  uintptr_t _thread_id;
  internal::thread_type _thread_type;
  internal::task_domain _task_domain;

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
  size_t _allocator_id;
#endif
};

extern tlsptr<info, storage::owner> *g_tls_info;

info* init();
void init_tls_info();
void shutdown_tls_info();
void error(std::string msg, const char* filename, int lineno,
           const char* funcname);

inline info*
get()
{
    auto ti = g_tls_info->get();
    if (ti)
        return ti;

    return init();
}

};

inline uintptr_t
thread_id()
{
  auto ti = tls::get();
  return ti->thread_id();
}

inline void
set_main_thread()
{
  auto ti = tls::get();
  ti->set_main_thread();
}

inline void
set_symphony_thread()
{
  auto ti = tls::get();
  ti->set_symphony_thread();
}

inline void
set_task_domain(symphony::internal::task_domain td)
{
  auto ti = tls::get();
  ti->set_task_domain(td);
}

};
};
