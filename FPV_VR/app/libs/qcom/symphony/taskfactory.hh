// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/cpukernel.hh>
#include <symphony/gpukernel.hh>
#include <symphony/groupptr.hh>
#include <symphony/hexagonkernel.hh>
#include <symphony/taskptr.hh>

#include <symphony/internal/task/operators.hh>
#include <symphony/internal/util/templatemagic.hh>

namespace symphony {

struct do_not_collapse_t {};

const do_not_collapse_t do_not_collapse {};

template<typename Fn>
using collapsed_task_type = typename ::symphony::internal::task_factory<Fn>::collapsed_task_type;

template<typename Fn>
using non_collapsed_task_type =typename ::symphony::internal::task_factory<Fn>::non_collapsed_task_type;

template<typename ReturnType, typename... Args>
::symphony::task_ptr<ReturnType> create_value_task(Args&&... args) {
  auto t = ::symphony::internal::cputask_factory::create_value_task<ReturnType>(std::forward<Args>(args)...);
  return ::symphony::task_ptr<ReturnType>(t, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

template<typename Code, typename... Args>
collapsed_task_type<Code> create_task(Code&& code, Args&&... args)
{
  using factory = ::symphony::internal::task_factory<Code>;
  auto raw_task_ptr = factory::create_collapsed_task(std::forward<Code>(code),
                                                     std::forward<Args>(args)...);
  return collapsed_task_type<Code>(raw_task_ptr, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

namespace internal {
template<typename... Args>
void create_poly_task_impl(group*, task*, std::tuple<>&&, Args&&...)
{

}

template<typename Code1, typename... Codes, typename... Args>
void create_poly_task_impl(group* g, task* t, std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  using factory1 = task_factory<Code1>;
  auto raw_task_ptr1 = factory1::create_collapsed_task(std::forward<Code1>(std::get<0>(codes)),
                                                       std::forward<Args>(args)...);
  t->add_alternative(raw_task_ptr1);
  raw_task_ptr1->join_group(g, false);
  create_poly_task_impl(g, t, tuple_rest(codes), std::forward<Args>(args)...);
}

template<typename Code1, typename... Codes, typename... Args>
task* create_poly_task(std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  using factory1 = task_factory<Code1>;

  auto raw_task_ptr1 = factory1::create_collapsed_task(std::forward<Code1>(std::get<0>(codes)),
                                                       std::forward<Args>(args)...);

  auto g = symphony::create_group();
  auto g_ptr = c_ptr(g);

  create_poly_task_impl(g_ptr, raw_task_ptr1, tuple_rest(codes), std::forward<Args>(args)...);

  g_ptr->set_representative_task(raw_task_ptr1);

  return raw_task_ptr1;
}

template<typename... Args>
void create_non_collapsed_poly_task_impl(group*, task*, std::tuple<>&&, Args&&...)
{

}

template<typename Code1, typename... Codes, typename... Args>
void create_non_collapsed_poly_task_impl(group* g,
    task* t, std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  using factory1 = task_factory<Code1>;
  auto raw_task_ptr1 = factory1::create_non_collapsed_task(std::forward<Code1>(std::get<0>(codes)),
                                                           std::forward<Args>(args)...);
  t->add_alternative(raw_task_ptr1);
  raw_task_ptr1->join_group(g, false);
  create_non_collapsed_poly_task_impl(g, t, tuple_rest(codes), std::forward<Args>(args)...);
}

template<typename Code1, typename... Codes, typename... Args>
task* create_non_collapsed_poly_task(std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  using factory1 = task_factory<Code1>;

  auto raw_task_ptr1 = factory1::create_non_collapsed_task(std::forward<Code1>(std::get<0>(codes)),
                                                           std::forward<Args>(args)...);

  auto g = symphony::create_group();
  auto g_ptr = c_ptr(g);

  create_non_collapsed_poly_task_impl(g_ptr, raw_task_ptr1, tuple_rest(codes), std::forward<Args>(args)...);

  g_ptr->set_representative_task(raw_task_ptr1);

  return raw_task_ptr1;
}
};

namespace beta {

template<typename Code1, typename... Codes, typename... Args>
collapsed_task_type<Code1> create_task(std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  auto raw_task_ptr1 = ::symphony::internal::create_poly_task(std::move(codes), std::forward<Args>(args)...);
  return collapsed_task_type<Code1>(raw_task_ptr1, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}
};

template<typename Code, typename... Args>
non_collapsed_task_type<Code> create_task(do_not_collapse_t, Code&& code, Args&&... args)
{
  using factory = ::symphony::internal::task_factory<Code>;
  auto raw_task_ptr = factory::create_non_collapsed_task(std::forward<Code>(code),
                                                         std::forward<Args>(args)...);
  return non_collapsed_task_type<Code>(raw_task_ptr, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

namespace beta {

template<typename Code1, typename... Codes, typename... Args>
non_collapsed_task_type<Code1> create_task(do_not_collapse_t, std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  auto raw_task_ptr1 = ::symphony::internal::create_non_collapsed_poly_task(std::move(codes), std::forward<Args>(args)...);
  return non_collapsed_task_type<Code1>(raw_task_ptr1, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}
};

template<typename Code, typename...Args>
collapsed_task_type<Code> launch(Code&& code, Args&& ...args)
{
  using factory = ::symphony::internal::task_factory<Code>;
  auto raw_task_ptr = factory::create_collapsed_task(std::forward<Code>(code),
                                                     std::forward<Args>(args)...);
  raw_task_ptr->launch(nullptr, nullptr);
  return collapsed_task_type<Code>(raw_task_ptr, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

namespace beta {

template<typename Code1, typename... Codes, typename... Args>
collapsed_task_type<Code1> launch(std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  auto raw_task_ptr1 = ::symphony::internal::create_poly_task(std::move(codes), std::forward<Args>(args)...);
  raw_task_ptr1->launch(nullptr, nullptr);
  return collapsed_task_type<Code1>(raw_task_ptr1, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}
};

template<typename Code, typename...Args>
non_collapsed_task_type<Code> launch(do_not_collapse_t, Code&& code, Args&& ...args)
{
  using factory = ::symphony::internal::task_factory<Code>;
  auto raw_task_ptr = factory::create_non_collapsed_task(std::forward<Code>(code),
                                                         std::forward<Args>(args)...);
  raw_task_ptr->launch(nullptr, nullptr);
  return non_collapsed_task_type<Code>(raw_task_ptr, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}

namespace beta {

template<typename Code1, typename... Codes, typename... Args>
non_collapsed_task_type<Code1> launch(do_not_collapse_t, std::tuple<Code1, Codes...>&& codes, Args&&... args)
{
  auto raw_task_ptr1 = ::symphony::internal::create_non_collapsed_poly_task(std::move(codes), std::forward<Args>(args)...);
  raw_task_ptr1->launch(nullptr, nullptr);
  return non_collapsed_task_type<Code1>(raw_task_ptr1, ::symphony::internal::task_shared_ptr::ref_policy::no_initial_ref);
}
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

SYMPHONY_UNARY_TASKPTR_OP(+)
SYMPHONY_UNARY_TASKPTR_OP(-)
SYMPHONY_UNARY_TASKPTR_OP(~)

SYMPHONY_BINARY_TASKPTR_OP_2PTRS(+)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(-)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(*)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(/)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(%)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(&)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(|)
SYMPHONY_BINARY_TASKPTR_OP_2PTRS(^)

SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(+)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(-)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(*)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(/)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(%)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(&)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(|)
SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(^)

SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(+)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(-)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(*)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(/)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(%)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(&)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(|)
SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(^)

SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(+)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(-)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(*)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(/)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(%)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(&)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(|)
SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(^)

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
