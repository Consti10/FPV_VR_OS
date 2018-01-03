// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/taskfactory.hh>
#include <symphony/tuner.hh>

#include <symphony/internal/patterns/pscan.hh>

namespace symphony {
namespace pattern {

template <typename BinaryFn> class pscan;

template <typename BinaryFn>
pscan<BinaryFn> create_pscan_inclusive(BinaryFn&& fn);

template <typename BinaryFn>
class pscan {
public:
  template <typename RandomAccessIterator>
  void run(RandomAccessIterator first,
           RandomAccessIterator last,
           symphony::pattern::tuner t = symphony::pattern::tuner()) {
    if (!t.is_chunk_set()) {
      t.set_chunk_size(1024);
    }

    symphony::internal::pscan_inclusive_internal
      (nullptr, first, last, std::forward<BinaryFn>(_fn), t);
  }

  template <typename RandomAccessIterator>
  void operator()(RandomAccessIterator first,
                  RandomAccessIterator last,
                  const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    run(first, last, t);
  }

private:
  BinaryFn _fn;
  explicit pscan(BinaryFn&& fn) : _fn(fn) {}
  friend pscan create_pscan_inclusive<BinaryFn>(BinaryFn&& fn);
  template<typename Fn, typename ...Args>
  friend symphony::task_ptr<void()> symphony::create_task(const pscan<Fn>& ps, Args&&...args);
};

template <typename BinaryFn>
pscan<BinaryFn>
create_pscan_inclusive(BinaryFn&& fn) {
  using traits = internal::function_traits<BinaryFn>;

  static_assert(traits::arity::value == 2,
      "pscan takes a function accepting two arguments");

  return pscan<BinaryFn>(std::forward<BinaryFn>(fn));
}

};

template <typename RandomAccessIterator, typename BinaryFn>
void pscan_inclusive(RandomAccessIterator first,
                     RandomAccessIterator last,
                     BinaryFn&& fn,
                     symphony::pattern::tuner tuner = symphony::pattern::tuner()) {

  if (!tuner.is_chunk_set()) {
    tuner.set_chunk_size(1024);
  }

  symphony::internal::pscan_inclusive_internal
    (nullptr, first, last, std::forward<BinaryFn>(fn), tuner);
}

template <typename RandomAccessIterator, typename BinaryFn>
symphony::task_ptr<void()> pscan_inclusive_async
       (RandomAccessIterator first,
        RandomAccessIterator last,
        BinaryFn fn,
        symphony::pattern::tuner tuner = symphony::pattern::tuner()) {
  if (!tuner.is_chunk_set()) {
    tuner.set_chunk_size(1024);
  }

  return symphony::internal::pscan_inclusive_async(std::forward<BinaryFn>(fn), first, last, tuner);
}

template <typename BinaryFn, typename ...Args>
symphony::task_ptr<void()>
create_task(const symphony::pattern::pscan<BinaryFn>& ps, Args&&...args)
{
  return symphony::internal::pscan_inclusive_async(ps._fn, args...);
}

template <typename BinaryFn, typename ...Args>
symphony::task_ptr<void()> launch(symphony::pattern::pscan<BinaryFn>& ps, Args&&...args)
{
  auto t = symphony::create_task(ps, args...);
  t->launch();
  return t;
}

};
