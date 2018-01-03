// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/taskfactory.hh>
#include <symphony/tuner.hh>

#include <symphony/internal/patterns/psort.hh>

namespace symphony {
namespace pattern {

template <typename Compare> class psorter;

template <typename Compare> psorter<Compare> create_psort(Compare&& cmp);

template <typename Compare>
class psorter {
public:
  template <typename RandomAccessIterator>
  void run(RandomAccessIterator first, RandomAccessIterator last, const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    internal::psort_internal(first, last, _cmp, t);
  }

  template <typename RandomAccessIterator>
  void operator()(RandomAccessIterator first, RandomAccessIterator last,
                  const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    run(first, last, t);
  }

  explicit psorter(Compare&& cmp) : _cmp(cmp) {}

private:
  Compare _cmp;
  friend psorter create_psort<Compare>(Compare&& cmp);
  template<typename Cmp, typename ...Args>
  friend symphony::task_ptr<void()> symphony::create_task(const psorter<Cmp>& p, Args&&...args);
};

template <typename Compare>
psorter<Compare> create_psort(Compare&& cmp) {
  using traits = internal::function_traits<Compare>;

  static_assert(traits::arity::value == 2, "psort takes a function accepting two arguments");

  return psorter<Compare>(std::forward<Compare>(cmp));
}

};

template <class RandomAccessIterator, class Compare>
void psort(RandomAccessIterator first, RandomAccessIterator last, Compare cmp,
    const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto p = symphony::pattern::create_psort(cmp);
  p(first, last, tuner);
}

template <class RandomAccessIterator, class Compare>
symphony::task_ptr<void()>
psort_async(RandomAccessIterator first, RandomAccessIterator last, Compare&& cmp,
    const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  return symphony::internal::psort_async
    (std::forward<Compare>(cmp), first, last, tuner);
}

template <class RandomAccessIterator>
void psort(RandomAccessIterator first, RandomAccessIterator last,
    const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto p = symphony::pattern::create_psort(std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>());
  p(first, last, tuner);
}

template <class RandomAccessIterator>
symphony::task_ptr<void()>
psort_async(RandomAccessIterator first, RandomAccessIterator last,
    const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto g = internal::legacy::create_group();
  auto t = symphony::create_task([g, first, last, tuner]{
      internal::psort_internal(first, last, std::less<typename std::iterator_traits<RandomAccessIterator>::value_type>(), tuner);
      internal::legacy::finish_after(g);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <typename Compare, typename ...Args>
symphony::task_ptr<void()>
create_task(const symphony::pattern::psorter<Compare>& p, Args&&...args)
{
  return symphony::internal::psort_async(p._cmp, args...);
}

template <typename Compare, typename ...Args>
symphony::task_ptr<void()>
launch(const symphony::pattern::psorter<Compare>& p, Args&&...args)
{
  auto t = symphony::create_task(p, args...);
  t->launch();
  return t;
}

};
