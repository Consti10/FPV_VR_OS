// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/group.hh>

#include <symphony/internal/task/group.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/task/taskfactory.hh>

namespace symphony {
namespace internal {

inline
::symphony::internal::group * c_ptr(::symphony::group* g) {
  return g->get_raw_ptr();
}

namespace group_launch {

template<typename Code, typename... Args>
struct launch_code {

  using decayed_type = typename std::decay<Code>::type;
  using is_pattern = typename std::conditional<internal::is_pattern<decayed_type>::value, std::true_type, std::false_type>::type;

  static_assert(::symphony::internal::is_symphony_task20_ptr<decayed_type>::value == false,
                "Invalid launch for Code");

  static_assert(::symphony::internal::is_group_launchable<decayed_type>::value == true,
                "Group launch is not supported due to non-void return value of the pattern.");

  template<typename CodeType = Code, typename T>
  static void launch_impl(bool notify, ::symphony::internal::group* gptr, CodeType&& code, T&&, Args&&... args){

    using factory = ::symphony::internal::task_factory<CodeType, is_pattern>;

    factory::launch(notify, gptr, std::forward<CodeType>(code), std::forward<Args>(args)...);
  }

};

template<typename T, typename...Args>
struct launch_task {

  using decayed_type = typename std::decay<T>::type;

  static_assert(::symphony::internal::is_symphony_task20_ptr<decayed_type>::value,
                "Invalid launch for tasks");

  static void launch_impl(bool notify,
                            ::symphony::internal::group* gptr,
                            T&& t,
                            std::true_type,
                            Args&&... args) {
    SYMPHONY_UNUSED(notify);

    static_assert(is_symphony_task20_ptr<decayed_type>::has_arguments == true,
                    "Invalid number of arguments for launching a task by the group.");

    auto ptr = ::symphony::internal::get_cptr(t);

    SYMPHONY_INTERNAL_ASSERT(ptr != nullptr, "Unexpected null task<>.");

    t->bind_all(std::forward<Args>(args)...);
    ptr->launch(gptr, nullptr);
  }

  static void launch_impl(bool notify,
                            ::symphony::internal::group* gptr,
                            T&& t,
                            std::false_type,
                            Args&&... ) {
    SYMPHONY_UNUSED(notify);

    static_assert(sizeof...(Args) == 0,
                  "Invalid number of arguments for launching a task by the group.");

    task* ptr = ::symphony::internal::get_cptr(t);
    SYMPHONY_INTERNAL_ASSERT(ptr != nullptr, "Unexpected null task<>.");

    ptr->launch(gptr, nullptr);
  }
};

};

namespace group_intersect {

  inline
  internal::group_shared_ptr
  intersect_impl(internal::group* a, internal::group* b)
  {

    if (a == nullptr || b == nullptr )
      return nullptr;

    if (a->is_ancestor_of(b))
      return internal::group_shared_ptr(b);

    if (b->is_ancestor_of(a))
      return internal::group_shared_ptr(a);

    return internal::group_factory::merge_groups(a, b);
  }

};

};

inline void
group::add(task_ptr<> const& task)
{
  SYMPHONY_INTERNAL_ASSERT(get_raw_ptr() != nullptr, "Unexpected null group.");
  auto t = internal::c_ptr(task);
  SYMPHONY_API_ASSERT(t, "null task pointer.");
  t->join_group(get_raw_ptr(), true);
}

template<typename TaskPtrOrCode, typename...Args>
void
group::launch(TaskPtrOrCode&& task_or_code, Args&& ...args)
{
  auto g = get_raw_ptr();
  SYMPHONY_INTERNAL_ASSERT(g != nullptr, "Unexpected null group.");

  using decayed_type = typename std::decay<TaskPtrOrCode>::type;

  using launch_policy = typename std::conditional<
    internal::is_symphony_task20_ptr<decayed_type>::value,
    internal::group_launch::launch_task<TaskPtrOrCode, Args...> ,
    internal::group_launch::launch_code<TaskPtrOrCode, Args...> >::type;

  using has_args = typename std::conditional<
    sizeof...(Args) == 0,
    std::false_type,
    std::true_type>::type;

  launch_policy::launch_impl(true,
                             g,
                             std::forward<TaskPtrOrCode>(task_or_code),
                             has_args(),
                             std::forward<Args>(args)...);
}

};
