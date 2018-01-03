// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/pfor_each.hh>

namespace symphony {

namespace internal {

template <typename RandomAccessIterator,
          typename T,
          typename TransformFn,
          typename BinaryFn>
void pscan_inclusive_adaptive(group* group,
                              RandomAccessIterator base,
                              size_t first,
                              size_t last,
                              TransformFn&& tfn,
                              BinaryFn&& fn,
                              const symphony::pattern::tuner& tuner)
{
  if (first >= last)
    return ;

  typedef symphony::internal::legacy::body_wrapper<TransformFn> scan_body_type;
  scan_body_type scan_wrapper(std::forward<TransformFn>(tfn));

  auto body_attrs = scan_wrapper.get_attrs();
  body_attrs = legacy::add_attr(body_attrs, symphony::internal::attr::inlined);

  SYMPHONY_INTERNAL_ASSERT(scan_body_type::get_arity() == 2,
                       "Invalid number of arguments in scan function");

  const size_t blk_size = tuner.get_chunk_size();
  auto g_internal = create_group();
  auto g = g_internal;
  if (group != nullptr)
    g = pattern::group_ptr_shim::shared_to_group_ptr_cast(group_intersect::intersect_impl(c_ptr(g), group));

  typedef adaptive_steal_strategy
    <T, RandomAccessIterator, TransformFn, BinaryFn, Policy::SCAN> strategy_type;
  symphony_shared_ptr<strategy_type> strategy_ptr
    (new strategy_type(g, base, first, last - 1,
                       std::forward<TransformFn>(tfn),
                       std::forward<BinaryFn>(fn),
                       body_attrs,
                       blk_size,
                       tuner));

  size_t max_tasks = strategy_ptr->get_max_tasks();
  if (max_tasks > 1 && max_tasks < static_cast<size_t>(last - first))
     strategy_ptr->static_split(max_tasks);

  execute_master_task<T, strategy_type, Policy::SCAN>(strategy_ptr);
  spin_wait_for(g);
}

template <typename RandomAccessIterator, typename BinaryFn>
void pscan_inclusive_internal(group* group,
                              RandomAccessIterator first,
                              RandomAccessIterator last,
                              BinaryFn fn,
                              const symphony::pattern::tuner& tuner)
{

  if (tuner.is_serial())
  {
    for (RandomAccessIterator idx =  first + 1; idx < last; idx++) {
      *idx = fn(*(idx - 1), *idx);
    }
    return ;
  }

  else{

    auto fn_transform = [fn, first](size_t idx1, size_t idx2) {
      *(first + idx2) = fn(*(first + idx1), *(first + idx2));
    };

    typedef typename std::iterator_traits<RandomAccessIterator>::value_type vtype;
      pscan_inclusive_adaptive<RandomAccessIterator, vtype>
        (group, first, 0, last - first, fn_transform, fn, tuner);
  }
}

template <typename RandomAccessIterator, typename BinaryFn>
symphony::task_ptr<void()> pscan_inclusive_async(
        BinaryFn fn,
        RandomAccessIterator first,
        RandomAccessIterator last,
        const symphony::pattern::tuner& tuner = symphony::pattern::tuner()) {
  auto g = create_group();
  auto t = symphony::create_task([g, first, last, fn, tuner]{

    symphony::internal::pscan_inclusive_internal(c_ptr(g), first, last, fn, tuner);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

};
};
