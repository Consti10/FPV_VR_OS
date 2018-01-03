// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/runtime.hh>
#include <symphony/internal/task/hexagontask.hh>
#include <symphony/internal/task/hexagontaskfactory.hh>
#include <symphony/internal/task/hexagontraits.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/macros.hh>

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
#include <symphony/internal/memalloc/concurrentbumppool.hh>
#include <symphony/internal/memalloc/taskallocator.hh>
#endif

namespace symphony {

template<typename Fn> class hexagon_kernel;

namespace internal {

class group;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

struct hexagontask_factory {

  template<typename RawType, typename HexagonKernel, typename... Args>
  static RawType* create_task(group* g, legacy::task_attrs attrs, HexagonKernel&& hk, Args&&...args) {
    attrs = legacy::add_attr(attrs, attr::api20);

    auto initial_state = task::get_initial_state_bound();

    return create_task_impl<RawType, HexagonKernel, Args...>(initial_state,
                                                   g,
                                                   attrs,
                                                   std::forward<HexagonKernel>(hk),
                                                   std::forward<Args>(args)...);
  }

  template<typename RawType, typename HexagonKernel, typename... Args>
  static RawType* create_anonymous_task(group* g, legacy::task_attrs attrs, HexagonKernel&& hk, Args&& ...args) {
    attrs = legacy::add_attr(attrs, attr::api20);
    return create_task_impl<RawType, HexagonKernel, Args...>(task::get_initial_state_anonymous(),
                                                       g,
                                                       attrs,
                                                       std::forward<HexagonKernel>(hk),
                                                       std::forward<Args>(args)...);
  }

private:

  template<typename RawType, typename HexagonKernel, typename... Args>
  static RawType* create_task_impl(task::state_snapshot initial_state,
                                   group* g,
                                   legacy::task_attrs attrs,
                                   HexagonKernel&& hk,
                                   Args&& ...args) {

    return new hexagontask<HexagonKernel, Args...>(initial_state,
                                         g,
                                         attrs,
                                         std::forward<HexagonKernel>(hk),
                                         std::forward<Args>(args)...);

  }
};

template<typename Fn>
struct task_factory_dispatch<::symphony::hexagon_kernel<Fn> > : public hexagontask_factory {

private:

  using kernel = typename ::symphony::hexagon_kernel<Fn>;
  using fn_type = typename kernel::fn_type;

protected:

  using collapsed_task_type = typename symphony::task_ptr<void(void)>;
  using non_collapsed_task_type = typename symphony::task_ptr<void(void)>;

  using collapsed_raw_task_type = symphony::internal::task;
  using non_collapsed_raw_task_type = symphony::internal::task;

  static fn_type&& get_fn(kernel&& k) {
    return static_cast<fn_type&&>(k.get_fn());
  }

  static fn_type& get_fn(kernel& k) {
    return static_cast<fn_type&>(k.get_fn());
  }

  template<typename HexagonKernel, typename... TaskArgs>
  static collapsed_raw_task_type* create_collapsed_task(HexagonKernel&& k, TaskArgs&&...args) {

    symphony::internal::check<typename std::remove_reference<HexagonKernel>::type, TaskArgs...>();

    auto t = create_task<
                       collapsed_raw_task_type,
                       HexagonKernel,
                       TaskArgs...>(nullptr,
                                    k.get_attrs(),
                                    std::forward<HexagonKernel>(k),
                                    std::forward<TaskArgs>(args)...);

    return t;
  }

  template<typename HexagonKernel, typename... TaskArgs>
  static non_collapsed_raw_task_type* create_non_collapsed_task(HexagonKernel&& k, TaskArgs&& ...args) {

    symphony::internal::check<typename std::remove_reference<HexagonKernel>::type, TaskArgs...>();

    auto t = create_task<
                       non_collapsed_raw_task_type,
                       HexagonKernel,
                       TaskArgs...>(nullptr,
                                                                      k.get_attrs(),
                                                                      std::forward<HexagonKernel>(k),
                                                                      std::forward<TaskArgs>(args)...);
    return t;
  }

  template<typename HexagonKernel, typename... TaskArgs>
  static bool launch(bool notify, group* g ,  HexagonKernel&& k, TaskArgs&& ...args) {

    symphony::internal::check<typename std::remove_reference<HexagonKernel>::type, TaskArgs...>();

    auto attrs = k.get_attrs();

    SYMPHONY_INTERNAL_ASSERT(has_attr(attrs, ::symphony::internal::legacy::attr::blocking) == false,
                         "Hexagon task cannot be blocking");

    k.set_attr(attr::anonymous);

    task* t = create_anonymous_task<non_collapsed_raw_task_type>(g,
                                                                k.get_attrs(),
                                                                std::forward<HexagonKernel>(k),
                                                                std::forward<TaskArgs>(args)...);
    if (t == nullptr)
      return false;

    send_to_runtime(t, nullptr, notify);
    return true;
  }
};
#endif

SYMPHONY_GCC_IGNORE_END("-Weffc++");
};
};
