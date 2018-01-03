// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/common.hh>

namespace symphony {

namespace internal {

template <typename InputIterator, typename OutputIterator, typename UnaryFn>
void ptransform_static(group* group,
                       InputIterator first,
                       InputIterator last,
                       OutputIterator d_first,
                       UnaryFn fn,
                       const symphony::pattern::tuner& tuner) {

  if (first >= last)
    return;

  auto g_internal = create_group();
  auto g = g_internal;
  if (group != nullptr)
    g = pattern::group_ptr_shim::shared_to_group_ptr_cast(group_intersect::intersect_impl(c_ptr(g), group));

  size_t const dist = std::distance(first,last);
  auto blksz = internal::static_chunk_size(dist, tuner.get_doc());
  SYMPHONY_INTERNAL_ASSERT(blksz > 0, "block size should be greater than zero");
  auto lb = first;
  auto d_lb = d_first;
  while (lb < last) {
    auto safeofs = std::min(size_t(std::distance(lb,last)), blksz);
    InputIterator rb = lb + safeofs;

    auto gs = pattern::group_ptr_shim::group_to_shared_ptr_cast(g);
    launch_or_exec(rb != last, gs, [=] {
        InputIterator it = lb;
        OutputIterator d_it = d_lb;
        while (it < rb)
          *d_it++ = fn(*it++);
      });
    lb = rb;
    d_lb += safeofs;
  }

  spin_wait_for(g);
}

template <typename InputIterator, typename OutputIterator, typename BinaryFn>
void ptransform_static(group* group,
                       InputIterator first1,
                       InputIterator last1,
                       InputIterator first2,
                       OutputIterator d_first,
                       BinaryFn fn,
                       const symphony::pattern::tuner& tuner) {
  if (first1 >= last1)
    return;

  auto g_internal = create_group();
  auto g = g_internal;
  if (group != nullptr)
    g = pattern::group_ptr_shim::shared_to_group_ptr_cast(group_intersect::intersect_impl(c_ptr(g), group));

  size_t const dist = std::distance(first1,last1);
  auto blksz = internal::static_chunk_size(dist, tuner.get_doc());
  SYMPHONY_INTERNAL_ASSERT(blksz > 0, "block size should be greater than zero");
  auto lb = first1;
  auto lb2 = first2;
  auto d_lb = d_first;
  while (lb < last1) {
    auto safeofs = std::min(size_t(std::distance(lb,last1)), blksz);
    InputIterator rb = lb + safeofs;

    auto gs = pattern::group_ptr_shim::group_to_shared_ptr_cast(g);
    launch_or_exec(rb != last1, gs, [=] {
        InputIterator it = lb;
        InputIterator it2 = lb2;
        OutputIterator d_it = d_lb;
        while (it < rb)
          *d_it++ = fn(*it++, *it2++);
      });
    lb = rb;
    lb2 += safeofs;
    d_lb += safeofs;
  }

  spin_wait_for(g);
}

template<typename InputIterator, typename OutputIterator, typename UnaryFn>
void ptransform_dynamic(group* g,
                        InputIterator first,
                        InputIterator last,
                        OutputIterator d_first,
                        UnaryFn fn,
                        const symphony::pattern::tuner& tuner)
{
  auto new_fn = [first, d_first, fn] (size_t i) {
    *(d_first + i) = fn(*(first + i));
  };

  internal::pfor_each_dynamic(g, 0, std::distance(first, last), new_fn, 1, tuner);
}

template<typename InputIterator, typename OutputIterator, typename BinaryFn>
void ptransform_dynamic(group* g,
                        InputIterator first1,
                        InputIterator last1,
                        InputIterator first2,
                        OutputIterator d_first,
                        BinaryFn fn,
                        const symphony::pattern::tuner& tuner)
{
  auto new_fn = [first1, first2, d_first, fn] (size_t i) {
    *(d_first + i) = fn(*(first1 + i), *(first2 + i));
  };

  internal::pfor_each_dynamic(g, 0, std::distance(first1, last1), new_fn, 1, tuner);
}

template <typename InputIterator, typename OutputIterator, typename UnaryFn>
void ptransform(group* g,
                InputIterator first, InputIterator last,
                OutputIterator d_first, UnaryFn&& fn,
                const symphony::pattern::tuner& tuner) {

  SYMPHONY_API_ASSERT(symphony::internal::callable_object_is_mutable<UnaryFn>::value == false,
                      "Mutable functor is not allowed in symphony patterns!");

  if (tuner.is_serial()) {
    InputIterator  it = first;
    OutputIterator d_it = d_first;
    while (it != last) {
      *(d_it++) = fn(*it++);
    }

    return ;
  }

  if (tuner.is_static()) {
    internal::ptransform_static(g, first, last, d_first, fn, tuner);
    return ;
  }

  internal::ptransform_dynamic(g, first, last, d_first, fn, tuner);
}

template <typename InputIterator, typename OutputIterator, typename UnaryFn>
void ptransform(int, InputIterator, InputIterator, OutputIterator,
                UnaryFn&&, const symphony::pattern::tuner&) = delete;

template <typename InputIterator, typename OutputIterator, typename BinaryFn>
void ptransform(group* g,
                InputIterator first1, InputIterator last1,
                InputIterator first2, OutputIterator d_first,
                BinaryFn&& fn, const symphony::pattern::tuner& tuner) {

  SYMPHONY_API_ASSERT(symphony::internal::callable_object_is_mutable<BinaryFn>::value == false,
                      "Mutable functor is not allowed in symphony patterns!");

  if (tuner.is_serial()) {
    InputIterator  it1 = first1;
    InputIterator  it2 = first2;
    OutputIterator d_it = d_first;
    while (it1 != last1) {
      *(d_it++) = fn(*it1++, *it2++);
    }

    return ;
  }

  if (tuner.is_static()) {
    internal::ptransform_static(g, first1, last1, first2, d_first, fn, tuner);
    return ;
  }

  internal::ptransform_dynamic(g, first1, last1, first2, d_first, fn, tuner);
}

template <typename InputIterator, typename OutputIterator, typename BinaryFn>
void ptransform(int, InputIterator, InputIterator, InputIterator,
                OutputIterator, BinaryFn&&, const symphony::pattern::tuner&) = delete;

template <typename InputIterator, typename UnaryFn>
void ptransform(group* g,
                InputIterator first, InputIterator last,
                UnaryFn fn, const symphony::pattern::tuner& tuner) {

  SYMPHONY_API_ASSERT(symphony::internal::callable_object_is_mutable<UnaryFn>::value == false,
                      "Mutable functor is not allowed in symphony patterns!");

  if (tuner.is_serial()) {
    for (InputIterator it = first; it != last; ++it) {
      fn(*it);
    }

    return ;
  }

  if (tuner.is_static()) {
    internal::pfor_each_static(g, first, last,
      [fn] (InputIterator it) { fn(*it); }, 1, tuner);
    return ;
  }

  internal::pfor_each_dynamic(g, 0, last - first,
    [first, fn] (size_t i) { fn(*(first + i)); }, 1, tuner);
}

template <typename InputIterator, typename UnaryFn>
void ptransform(int, InputIterator, InputIterator, UnaryFn&&,
                const symphony::pattern::tuner&) = delete;

template <typename InputIterator, typename OutputIterator, typename UnaryFn>
symphony::task_ptr<void()>
ptransform_async(UnaryFn&& fn, InputIterator first, InputIterator last,
                OutputIterator d_first,
                const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {

  auto g = create_group();
  auto t = symphony::create_task([g, first, last, d_first, fn, tuner]{
      internal::ptransform(c_ptr(g), first, last, d_first, fn, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <typename InputIterator, typename OutputIterator, typename BinaryFn>
symphony::task_ptr<void()>
ptransform_async(BinaryFn&& fn,
                InputIterator first1, InputIterator last1,
                InputIterator first2,
                OutputIterator d_first,
                const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  auto g = create_group();
  auto t = symphony::create_task([g, first1, last1, first2, d_first, fn, tuner]{
      internal::ptransform(c_ptr(g), first1, last1, first2, d_first, fn, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <typename InputIterator, typename UnaryFn>
symphony::task_ptr<void()>
ptransform_async(UnaryFn&& fn, InputIterator first, InputIterator last,
    const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  auto g = create_group();
  auto t = symphony::create_task([g, first, last, fn, tuner]{
      internal::ptransform(c_ptr(g), first, last, fn, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

};
};
