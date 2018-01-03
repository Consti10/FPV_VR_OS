// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/pfor_each.hh>
#include <symphony/taskfactory.hh>
#include <symphony/tuner.hh>

#include <symphony/internal/patterns/ptransform.hh>

namespace symphony {
namespace pattern {

template <typename Fn> class ptransformer;

template <typename Fn>
ptransformer<Fn> create_ptransform(Fn&& fn);

template <typename Fn>
class ptransformer {
public:
  template <typename InputIterator>
  void run(InputIterator first,
           InputIterator last,
           const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    internal::ptransform(nullptr, first, last, std::forward<Fn>(_fn), t);
  }

  template <typename InputIterator>
  void operator()(InputIterator first,
                  InputIterator last,
                  const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    run(first, last, t);
  }

  template <typename InputIterator, typename OutputIterator>
  void run(InputIterator first, InputIterator last, OutputIterator d_first,
           const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    internal::ptransform(nullptr, first, last, d_first, std::forward<Fn>(_fn), t);
  }

  template <typename InputIterator, typename OutputIterator>
  void operator()(InputIterator first, InputIterator last,
                  OutputIterator d_first,
                  const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    run(first, last, d_first, t);
  }

  template <typename InputIterator1, typename InputIterator2,
            typename OutputIterator>
  void run(InputIterator1 first1,
           InputIterator1 last1,
           InputIterator2 first2,
           OutputIterator d_first,
           const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    internal::ptransform(nullptr, first1, last1, first2, d_first, std::forward<Fn>(_fn), t);
  }

  template <typename InputIterator1, typename InputIterator2,
            typename OutputIterator>
  void operator()(InputIterator1 first1,
                  InputIterator1 last1,
                  InputIterator2 first2,
                  OutputIterator d_first,
                  const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    run(first1, last1, first2, d_first, t);
  }

private:
  Fn _fn;
  explicit ptransformer(Fn&& fn) : _fn(fn) {}
  friend ptransformer create_ptransform<Fn>(Fn&& fn);
  template<typename F, typename ...Args>
  friend symphony::task_ptr<void()> symphony::create_task(const ptransformer<F>& ptf, Args&&...args);
};

template <typename Fn>
ptransformer<Fn>
create_ptransform(Fn&& fn) {
  using traits = internal::function_traits<Fn>;

  static_assert(traits::arity::value == 1 || traits::arity::value == 2,
      "ptransform takes a function accepting either one or two arguments");

  return ptransformer<Fn>(std::forward<Fn>(fn));
}

};

template <typename InputIterator, typename OutputIterator, typename UnaryFn>

typename std::enable_if<!std::is_same<symphony::pattern::tuner, typename std::remove_reference<UnaryFn>
                        ::type>::value, void>::type
ptransform(InputIterator first,
           InputIterator last,
           OutputIterator d_first,
           UnaryFn&& fn,
           const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  internal::ptransform(nullptr, first, last, d_first, std::forward<UnaryFn>(fn), tuner);
}

template <typename InputIterator, typename OutputIterator, typename UnaryFn>
symphony::task_ptr<void()>
ptransform_async(InputIterator first, InputIterator last,
                 OutputIterator d_first, UnaryFn&& fn,
                 const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  return
    symphony::internal::ptransform_async(std::forward<UnaryFn>(fn), first, last, d_first, tuner);
}

template <typename InputIterator, typename OutputIterator, typename BinaryFn>

typename std::enable_if<!std::is_same<symphony::pattern::tuner, typename std::remove_reference<BinaryFn>
                        ::type>::value, void>::type
ptransform(InputIterator first1,
           InputIterator last1,
           InputIterator first2,
           OutputIterator d_first,
           BinaryFn&& fn,
           const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  internal::ptransform(nullptr, first1, last1, first2, d_first, std::forward<BinaryFn>(fn), tuner);
}

template <typename InputIterator, typename OutputIterator, typename BinaryFn>
symphony::task_ptr<void()>
ptransform_async(InputIterator first1, InputIterator last1,
                 InputIterator first2,
                 OutputIterator d_first, BinaryFn&& fn,
                 const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  return
    symphony::internal::ptransform_async(std::forward<BinaryFn>(fn), first1, last1, first2, d_first, tuner);
}

template <typename InputIterator, typename UnaryFn>
void ptransform(InputIterator first, InputIterator last, UnaryFn&& fn,
                const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  internal::ptransform(nullptr, first, last, std::forward<UnaryFn>(fn), tuner);
}

template <typename InputIterator, typename UnaryFn>
symphony::task_ptr<void()>
ptransform_async(InputIterator first, InputIterator last, UnaryFn&& fn,
                 const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  return symphony::internal::ptransform_async(std::forward<UnaryFn>(fn), first, last, tuner);
}

template <typename Fn, typename ...Args>
symphony::task_ptr<void()>
create_task(const symphony::pattern::ptransformer<Fn>& ptf, Args&&...args) {
  return symphony::internal::ptransform_async(ptf._fn, args...);
}

template <typename Fn, typename ...Args>
symphony::task_ptr<void()>
launch(const symphony::pattern::ptransformer<Fn>& ptf, Args&&...args) {
  auto t = symphony::create_task(ptf, args...);
  t->launch();
  return t;
}

};
