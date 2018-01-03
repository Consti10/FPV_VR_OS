// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/task/task.hh>

namespace symphony {
namespace internal {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

#define SYMPHONY_UNARY_TASKPTR_OP(op)                                                         \
 template<typename T>                                                                     \
 inline                                                                                   \
 ::symphony::task_ptr<typename ::symphony::task_ptr<T>::return_type>                              \
  operator op(const ::symphony::task_ptr<T>& t) {                                             \
    using op1_type = typename ::symphony::task_ptr<T>::return_type;                           \
    using op1_decayed_type = typename std::decay<op1_type>::type;                         \
    static_assert(::symphony::internal::is_symphony_task20_ptr<op1_decayed_type>::value == false, \
                  "Cannot perform " #op " operation on non-collapsed tasks.");            \
    auto c = ::symphony::create_task([](op1_type op1) {                                       \
        return op op1;                                                                    \
      });                                                                                 \
    c->bind_all(t);                                                                       \
    c->launch();                                                                          \
    return c;                                                                             \
  }

#define SYMPHONY_BINARY_TASKPTR_OP_2PTRS(op)                                                             \
template<typename T1, typename T2>                                                                   \
inline                                                                                               \
::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()                 \
                                 op                                                                  \
                                 std::declval<typename ::symphony::task_ptr<T2>::return_type>())>        \
operator op(const ::symphony::task_ptr<T1>& t1,                                                          \
            const ::symphony::task_ptr<T2>& t2) {                                                        \
  using op1_type = typename ::symphony::task_ptr<T1>::return_type;                                       \
  using op2_type = typename ::symphony::task_ptr<T2>::return_type;                                       \
  using op1_decayed_type = typename std::decay<op1_type>::type;                                      \
  using op2_decayed_type = typename std::decay<op2_type>::type;                                      \
  static_assert(::symphony::internal::is_symphony_task20_ptr<op1_decayed_type>::value == false,              \
                "Cannot perform " #op " operation on non-collapsed tasks.");                         \
  static_assert(::symphony::internal::is_symphony_task20_ptr<op2_decayed_type>::value == false,              \
                "Cannot perform " #op " operation on non-collapsed tasks.");                         \
  auto c = ::symphony::create_task([](op1_type op1, op2_type op2) {                                      \
      return op1 op op2;                                                                             \
  });                                                                                                \
  c->bind_all(t1, t2);                                                                               \
  c->launch();                                                                                       \
  return c;                                                                                          \
}

#define SYMPHONY_BINARY_TASKPTR_OP_PTR_VALUE(op)                                                         \
template<typename T1, typename T2>                                                                   \
inline typename                                                                                      \
std::enable_if<                                                                                      \
  ::symphony::internal::is_symphony_task20_ptr<typename std::decay<T2>::type>::value == false,               \
  ::symphony::task_ptr<decltype(std::declval<typename ::symphony::task_ptr<T1>::return_type>()               \
                          op                                                                         \
                          std::declval<T2>())>>::type                                                \
operator op(const ::symphony::task_ptr<T1>& t1, T2&& op2) {                                              \
  using op1_type = typename ::symphony::task_ptr<T1>::return_type;                                       \
  using op1_decayed_type = typename std::decay<op1_type>::type;                                      \
  static_assert(::symphony::internal::is_symphony_task20_ptr<op1_decayed_type>::value == false,              \
                "Cannot perform " #op " operation on non-collapsed tasks.");                         \
  auto c = ::symphony::create_task([op2](op1_type op1) {                                                 \
      return op1 op op2;                                                                             \
  });                                                                                                \
  c->bind_all(t1);                                                                                   \
  c->launch();                                                                                       \
  return c;                                                                                          \
}

#define SYMPHONY_BINARY_TASKPTR_OP_VALUE_PTR(op)                                                         \
template<typename T1, typename T2>                                                                   \
inline typename                                                                                      \
std::enable_if<                                                                                      \
  ::symphony::internal::is_symphony_task20_ptr<typename std::decay<T1>::type>::value == false,               \
  ::symphony::task_ptr<decltype(std::declval<T1>()                                                       \
                          op                                                                         \
                          std::declval<typename ::symphony::task_ptr<T2>::return_type>())>>::type        \
operator op(T1&& op1, const ::symphony::task_ptr<T2>& t2) {                                              \
  using op2_type = typename ::symphony::task_ptr<T2>::return_type;                                       \
  using op2_decayed_type = typename std::decay<op2_type>::type;                                      \
  static_assert(::symphony::internal::is_symphony_task20_ptr<op2_decayed_type>::value == false,              \
                "Cannot perform " #op " operation on non-collapsed tasks.");                         \
  auto c = ::symphony::create_task([op1](op2_type op2) {                                                 \
      return op1 op op2;                                                                             \
  });                                                                                                \
  c->bind_all(t2);                                                                                   \
  c->launch();                                                                                       \
  return c;                                                                                          \
}

#define SYMPHONY_TASKPTR_COMPOUND_ASSIGNMENT_OPERATOR(op)                                   \
template<typename ReturnType>                                                           \
  template<typename OpType>                                                             \
task_ptr<ReturnType>&                                                                   \
task_ptr<ReturnType>::                                                                  \
operator op##=(OpType&& opd) {                                                          \
  using decayed_optype = typename std::decay<OpType>::type;                             \
  static SYMPHONY_CONSTEXPR_CONST bool is_optype_taskptr =                                  \
    symphony::internal::is_symphony_task20_ptr<decayed_optype>::value;                          \
  using op1_type = ReturnType;                                                          \
  using op2_type =                                                                      \
    typename std::conditional<                                                          \
      is_optype_taskptr,                                                                \
      typename ::symphony::internal::is_symphony_task20_ptr<decayed_optype>::type,              \
      OpType>::type;                                                                    \
                                                                                        \
  using op1_decayed_type = typename std::decay<op1_type>::type;                         \
  using op2_decayed_type = typename std::decay<op2_type>::type;                         \
                                                                                        \
  static_assert(::symphony::internal::is_symphony_task20_ptr<op1_decayed_type>::value == false, \
                "Cannot perform " #op " operation on non-collapsed tasks.");            \
  static_assert(is_optype_taskptr == false ||                                           \
                ::symphony::internal::is_symphony_task20_ptr<op2_decayed_type>::value == false, \
                "Cannot perform " #op " operation on non-collapsed tasks.");            \
  auto t = ::symphony::create_task([](op1_type op1,                                         \
                                  op2_type op2) {                                       \
    return static_cast<op1_type>(op1 op op2);                                           \
  });                                                                                   \
  t->bind_all(*this, opd);                                                              \
  parent::operator=(std::move(t));                                                      \
  symphony::internal::task* t_ptr = get_raw_ptr();                                          \
  t_ptr->launch(nullptr, nullptr);                                                       \
  return *this;                                                                         \
}

SYMPHONY_GCC_IGNORE_END("-Weffc++");
};
};
