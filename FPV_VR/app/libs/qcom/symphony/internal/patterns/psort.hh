// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/pdivide_and_conquer.hh>

namespace symphony {

namespace internal {

template <typename RandomAccessIterator, typename Compare>
inline size_t median_of_three(const RandomAccessIterator& first, size_t a, size_t b, size_t c, Compare cmp) {
  return cmp(first[a], first[b]) ?
        (cmp(first[b], first[c]) ? b : cmp(first[a], first[c]) ? c : a)
      : (cmp(first[c], first[b]) ? b : cmp(first[c], first[a]) ? c : a);
}

template <typename RandomAccessIterator, typename Compare>
inline size_t ninther( const RandomAccessIterator& first, size_t n, Compare cmp) {
  auto size = n / 8;
  return median_of_three(first,
                          median_of_three(first, 0, size, size * 2, cmp),
                          median_of_three(first, size * 3, size * 4, size * 5, cmp),
                          median_of_three(first, size * 6, size * 7, n - 1, cmp),
                          cmp);

}

template <class RandomAccessIterator, class Compare>
void psort_internal(RandomAccessIterator first,
                    RandomAccessIterator last,
                    Compare cmp,
                    const symphony::pattern::tuner& tuner)
{
    if (tuner.is_serial()) {
      std::sort(first, last, cmp);
      return ;
    }

    struct quicksort_t {
      quicksort_t(RandomAccessIterator f, RandomAccessIterator l)
        : _first(f), _last(l), _middle() {}
      RandomAccessIterator _first, _last, _middle;
    };

    const size_t granularity = 512;

    symphony::internal::pdivide_and_conquer(
        nullptr,

        quicksort_t(first, last),

        [&](quicksort_t& q){
          size_t n = std::distance(q._first, q._last);
          if (n <= granularity) {
            return true;
          }

          auto pivot = ninther(q._first, n, cmp);
          q._middle = std::partition(q._first, q._last,
            std::bind(cmp, std::placeholders::_1, q._first[pivot]));

          return q._middle == q._first;
        },

        [&](quicksort_t& q){std::sort(q._first, q._last, cmp);},

        [&](quicksort_t& q){
          std::array<quicksort_t, 2> subarrays {{quicksort_t(q._first, q._middle),
              quicksort_t(q._middle, q._last)}};
          return subarrays;
        });
}

template <class RandomAccessIterator, class Compare>
symphony::task_ptr<void()>
psort_async(Compare&& cmp,
            RandomAccessIterator first,
            RandomAccessIterator last,
            const symphony::pattern::tuner& tuner = symphony::pattern::tuner())
{
  auto g = legacy::create_group();
  auto t = symphony::create_task([g, first, last, cmp, tuner]{
      internal::psort_internal(first, last, cmp, tuner);
      legacy::finish_after(g);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

};
};
