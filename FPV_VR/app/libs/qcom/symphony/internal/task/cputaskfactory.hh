// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/runtime.hh>
#include <symphony/internal/task/cputask.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/templatemagic.hh>

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
#include <symphony/internal/memalloc/concurrentbumppool.hh>
#include <symphony/internal/memalloc/taskallocator.hh>
#endif

namespace symphony {

template<typename Fn> class cpu_kernel;

namespace internal {

class group;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

struct cputask_factory {

  template<typename traits, typename RawType, typename TypeInfo, typename Fn, typename... Args>
  static RawType* create_task(group* g, legacy::task_attrs attrs, Fn&& fn, Args&&...args) {
    attrs = legacy::add_attr(attrs, attr::api20);
    auto initial_state = (traits::arity::value == sizeof...(Args))? task::get_initial_state_bound() :
                                                                    task::get_initial_state_unbound();

    return create_task_impl<RawType, TypeInfo, Fn>(initial_state,
                                                   g,
                                                   attrs,
                                                   std::forward<Fn>(fn),
                                                   std::forward<Args>(args)...);
  }

  template<typename RawType, typename TaskTypeInfo, typename Fn, typename... Args>
  static RawType* create_anonymous_task(group* g, legacy::task_attrs attrs, Fn&& fn, Args&& ...args) {
    attrs = legacy::add_attr(attrs, attr::api20);
    return create_task_impl<RawType, TaskTypeInfo, Fn>(task::get_initial_state_anonymous(),
                                                       g,
                                                       attrs,
                                                       std::forward<Fn>(fn),
                                                       std::forward<Args>(args)...);
  }

  template <typename ReturnType, typename... FnArgs>
  static value_cputask<ReturnType>* create_value_task(FnArgs&& ...args) {
    static auto s_default_cpu_attr = legacy::create_task_attrs(task_attr_cpu(), attr::api20);

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
    char* task_buffer = task_allocator::allocate(sizeof(value_cputask<ReturnType>));
    return new (task_buffer) value_cputask<ReturnType>(task::get_initial_state_value_task(),
                                                       nullptr,
                                                       s_default_cpu_attr,
                                                       std::forward<FnArgs>(args)...);
#else
    return new value_cputask<ReturnType>(task::get_initial_state_value_task(),
                                         nullptr,
                                         s_default_cpu_attr,
                                         std::forward<FnArgs>(args)...);
#endif
  }

private:

  template<typename RawType, typename TaskTypeInfo, typename Fn, typename... Args>
  static RawType* create_task_impl(task::state_snapshot initial_state,
                                   group* g,
                                   legacy::task_attrs attrs, Fn&& fn, Args&& ...args) {
#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
    char* buf = task_allocator::allocate(sizeof(cputask<TaskTypeInfo>));
    return new (buf) cputask<TaskTypeInfo>(initial_state, g, attrs, std::forward<Fn>(fn), std::forward<Args>(args)...);
#else
    return new cputask<TaskTypeInfo>(initial_state, g, attrs, std::forward<Fn>(fn), std::forward<Args>(args)...);
#endif
  }

};

template<typename Fn, class Enable = void> struct task_factory_dispatch;

template<typename Fn>
struct task_factory_dispatch<Fn> : public cputask_factory {

private:

  using kernel = typename ::symphony::cpu_kernel<Fn>;
  using traits = typename kernel::user_code_traits;

  using collapsed_task_type_info = typename kernel::collapsed_task_type_info;
  using non_collapsed_task_type_info = typename kernel::non_collapsed_task_type_info;

protected:

  using collapsed_task_type = typename kernel::collapsed_task_type;
  using non_collapsed_task_type = typename kernel::non_collapsed_task_type;

  using collapsed_raw_task_type = typename kernel::collapsed_raw_task_type;
  using non_collapsed_raw_task_type = typename kernel::non_collapsed_raw_task_type;

  template <typename UserFn, typename...Args>
  static collapsed_raw_task_type* create_collapsed_task(UserFn&& fn, Args&&...args) {
    auto default_cpu_attr = legacy::create_task_attrs(task_attr_cpu());
    return create_task<traits,
                       collapsed_raw_task_type,
                       collapsed_task_type_info>(nullptr,
                                                 default_cpu_attr,
                                                 std::forward<UserFn>(fn),
                                                 std::forward<Args>(args)...);

  }

  template <typename UserFn, typename...Args>
  static non_collapsed_raw_task_type* create_non_collapsed_task(UserFn&& fn, Args&&...args) {
    auto default_cpu_attr = legacy::create_task_attrs(task_attr_cpu());
    return create_task<traits,
                       non_collapsed_raw_task_type,
                       non_collapsed_task_type_info>(nullptr,
                                                     default_cpu_attr,
                                                     std::forward<UserFn>(fn),
                                                     std::forward<Args>(args)...);

  }

  template<typename UserFn, typename... Args>
  static bool launch(bool notify, group* g, UserFn&& fn, Args&&...args) {

    auto default_anonymous_cpu_attr = legacy::create_task_attrs(task_attr_cpu(), attr::anonymous);
    auto t = create_anonymous_task<non_collapsed_raw_task_type,
                                   non_collapsed_task_type_info> (g,
                                                               default_anonymous_cpu_attr,
                                                               std::forward<UserFn>(fn),
                                                               std::forward<Args>(args)...);
    if (t == nullptr)
      return false;

    send_to_runtime(t, nullptr, notify);
    return true;
  }
};

template<typename Fn>
struct task_factory_dispatch<::symphony::cpu_kernel<Fn>> : public cputask_factory {

private:

  using kernel = typename ::symphony::cpu_kernel<Fn>;
  using traits = typename kernel::user_code_traits;

  using collapsed_task_type_info = typename kernel::collapsed_task_type_info;
  using non_collapsed_task_type_info = typename kernel::non_collapsed_task_type_info;
  using fn_type = typename kernel::user_code_type_in_task;

protected:

  using collapsed_task_type = typename kernel::collapsed_task_type;
  using non_collapsed_task_type = typename kernel::non_collapsed_task_type;

  using collapsed_raw_task_type = typename kernel::collapsed_raw_task_type;
  using non_collapsed_raw_task_type = typename kernel::non_collapsed_raw_task_type;

  static fn_type&& get_fn(kernel&& k) {
    return static_cast<fn_type&&>(k.get_fn());
  }

  static fn_type& get_fn(kernel& k) {
    return static_cast<fn_type&>(k.get_fn());
  }

  template<typename CpuKernel, typename... Args>
  static collapsed_raw_task_type* create_collapsed_task(CpuKernel&& k, Args&&...args) {
    return create_task<traits,
                       collapsed_raw_task_type,
                       collapsed_task_type_info>(nullptr,
                                                                  k.get_attrs(),
                                                                  get_fn(std::forward<CpuKernel>(k)),
                                                                  std::forward<Args>(args)...);
  }

  template<typename CpuKernel, typename... Args>
  static non_collapsed_raw_task_type* create_non_collapsed_task(CpuKernel&& k, Args&& ...args) {
    return create_task<traits,
                       non_collapsed_raw_task_type,
                       non_collapsed_task_type_info>(nullptr,
                                                                      k.get_attrs(),
                                                                      get_fn(std::forward<CpuKernel>(k)),
                                                                      std::forward<Args>(args)...);
  }

  template<typename CpuKernel, typename... Args>
  static bool launch(bool notify, group* g, CpuKernel&& k, Args&& ...args) {

    auto attrs = k.get_attrs();

    if (has_attr(attrs, ::symphony::internal::legacy::attr::blocking) == false) {
      k.set_attr(attr::anonymous);
      task* t = create_anonymous_task<non_collapsed_raw_task_type,
                                      non_collapsed_task_type_info>(g,
                                                                    k.get_attrs(),
                                                                    get_fn(std::forward<CpuKernel>(k)),
                                                                    std::forward<Args>(args)...);
      if (t == nullptr)
        return false;

      send_to_runtime(t, nullptr, notify);
      return true;
    }

    task* t = create_task<traits,
                          non_collapsed_raw_task_type,
                          non_collapsed_task_type_info>(g,
                                                        k.get_attrs(),
                                                        get_fn(std::forward<CpuKernel>(k)),
                                                        std::forward<Args>(args)...);
    if (t == nullptr)
      return false;

    t->launch(nullptr, nullptr);
    return true;
  }
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
};
