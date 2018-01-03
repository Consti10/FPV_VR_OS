// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/task/cputaskfactory.hh>
#include <symphony/internal/task/gputaskfactory.hh>
#include <symphony/internal/task/group.hh>
#include <symphony/internal/task/hexagontaskfactory.hh>
#include <symphony/internal/util/macros.hh>

#if !defined(SYMPHONY_NO_TASK_ALLOCATOR)
#include <symphony/internal/memalloc/concurrentbumppool.hh>
#include <symphony/internal/memalloc/taskallocator.hh>
#endif

#include <symphony/tuner.hh>

namespace symphony {

template<typename Fn> class cpu_kernel;
template<typename Fn> class hexagon_kernel;

template<typename Fn, typename ...Args>
task_ptr<void()> create_task(const pattern::pfor<Fn, void>& pf, Args&&... args);

template<typename Fn, typename ...Args>
task_ptr<void()> create_task(const pattern::ptransformer<Fn>& ptf, Args&&... args);

template<typename BinaryFn, typename ...Args>
task_ptr<void()> create_task(const pattern::pscan<BinaryFn>& ps, Args&&... args);

template<typename Compare, typename ...Args>
task_ptr<void()> create_task(const pattern::psorter<Compare>& p, Args&&... args);

namespace internal {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template <typename Problem, typename Fn1, typename Fn2, typename Fn3, typename Fn4>
symphony::task_ptr<>
pdivide_and_conquer_async(Fn1&& is_base, Fn2&& base, Fn3&& split, Fn4&& merge, Problem p);

template <typename F>
struct is_pdnc : std::integral_constant<bool, false> {
};

template <typename Fn1, typename Fn2, typename Fn3, typename Fn4>
struct is_pdnc<::symphony::pattern::pdivide_and_conquerer<Fn1, Fn2, Fn3, Fn4>> :
  std::integral_constant<bool, true> {
};

template<typename F, class Enable = void>
struct is_taskable : std::integral_constant<bool, false> {
};

template<typename F>
struct is_taskable<F, typename std::enable_if<!std::is_same<typename function_traits<F>::kind,
                                                            user_code_types::invalid>::value>::type>
                  : std::integral_constant<bool, true> {
};

template<typename F>
struct is_taskable<::symphony::cpu_kernel<F>, void> : std::integral_constant<bool, true> {
};

template<typename F>
struct is_taskable<::symphony::hexagon_kernel<F>, void> : std::integral_constant<bool, true> {
};

template<typename ...KernelArgs>
struct is_taskable<::symphony::gpu_kernel<KernelArgs...>, void> : std::integral_constant<bool, true> {
};

template<typename Code, typename IsPattern = std::false_type, class Enable = void>
struct task_factory;

template<typename Code>
struct task_factory<Code, std::false_type,
                    typename std::enable_if<is_taskable<typename std::remove_reference<Code>::type>::value>::type>
  : public task_factory_dispatch<typename std::remove_reference<Code>::type> {

private:

  using parent = task_factory_dispatch<typename std::remove_reference<Code>::type>;

public:

  using collapsed_task_type = typename parent::collapsed_task_type;
  using non_collapsed_task_type = typename parent::non_collapsed_task_type;

  using collapsed_raw_task_type = typename parent::collapsed_raw_task_type;
  using non_collapsed_raw_task_type = typename parent::non_collapsed_raw_task_type;

  template<typename UserCode, typename... Args>
  static collapsed_raw_task_type* create_collapsed_task(UserCode&& code, Args&&...args) {
    return parent::create_collapsed_task(std::forward<UserCode>(code), std::forward<Args>(args)...);
  };

  template<typename UserCode, typename... Args>
  static non_collapsed_raw_task_type* create_non_collapsed_task(UserCode&& code, Args&&...args) {
    return parent::create_non_collapsed_task(std::forward<UserCode>(code), std::forward<Args>(args)...);
  };

  template<typename UserCode, typename... Args>
  static void launch(bool notify, group* g, UserCode&& code, Args&&...args) {
    g->inc_task_counter();
    bool success = parent::launch(notify, g, std::forward<UserCode>(code), std::forward<Args>(args)...);

    if (!success)
      g->dec_task_counter();
  }
};

template<typename Code>
struct task_factory<Code, std::true_type,
                    typename std::enable_if<!is_pdnc<typename std::decay<Code>::type>::value>::type>
{
  template<typename Pattern = Code, typename... Args>
  static void launch(bool notify, group* g, Pattern&& p, Args&&...args) {
    SYMPHONY_UNUSED(notify);

    auto t = ::symphony::create_task(std::forward<Pattern>(p), std::forward<Args>(args)...);
    auto task_ptr = ::symphony::internal::get_cptr(t);

    task_ptr->launch(g, nullptr);
  }
};

template <typename Code>
struct task_factory<Code, std::true_type,
                    typename std::enable_if<is_pdnc<typename std::decay<Code>::type>::value>::type>
{
  template<typename Pattern = Code, typename Problem>
  static void launch(bool notify, group* g, Pattern&& p, Problem pbl) {

    SYMPHONY_UNUSED(notify);

    auto t = ::symphony::internal::pdivide_and_conquer_async(p._is_base_fn,
                                                             p._base_fn,
                                                             p._split_fn,
                                                             p._merge_fn,
                                                             pbl);

    auto task_ptr = ::symphony::internal::get_cptr(t);
    task_ptr->launch(g, nullptr);
  }
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");
};
};
