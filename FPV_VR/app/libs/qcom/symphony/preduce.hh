// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/taskfactory.hh>
#include <symphony/tuner.hh>

#include <symphony/internal/patterns/preduce.hh>

namespace symphony {
namespace pattern {

template <typename Reduce, typename Join> class preducer;

template <typename Reduce, typename Join>
preducer<Reduce, Join> create_preduce(Reduce&& r, Join&& j);

template <typename Reduce, typename Join>
class preducer {
public:
  template <typename T, typename InputIterator>
  T run(InputIterator first,
        InputIterator last,
        const T& identity,
        const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    return internal::preduce_internal
              (nullptr, first, last, identity,
               std::forward<Reduce>(_reduce),
               std::forward<Join>(_join),
               t);
  }

  template <typename T, typename InputIterator>
  T operator()(InputIterator first, InputIterator last, const T& identity,
      const symphony::pattern::tuner& t = symphony::pattern::tuner()) {
    return run(first, last, identity, t);
  }

private:
  Reduce _reduce;
  Join   _join;
  preducer(Reduce&& r, Join&& j) : _reduce(r), _join(j) {}
  friend preducer create_preduce<Reduce, Join>(Reduce&& r, Join&& j);
  template<typename T, typename R, typename J, typename ...Args>
  friend symphony::task_ptr<T> symphony::create_task(const preducer<R, J>& p, Args&&...args);
};

template <typename Reduce, typename Join>
preducer<Reduce, Join> create_preduce(Reduce&& r, Join&& j) {
  return preducer<Reduce, Join>
    (std::forward<Reduce>(r), std::forward<Join>(j));
}

};

template<typename T, class InputIterator, typename Reduce, typename Join>
T preduce(InputIterator first,
          InputIterator last,
          const T& identity,
          Reduce&& reduce,
          Join&& join,
          symphony::pattern::tuner tuner = symphony::pattern::tuner())
{

  if (!tuner.is_chunk_set()) {
    auto chunk_size = symphony::internal::static_chunk_size<size_t>
                    (static_cast<size_t>(last - first),
                     symphony::internal::num_execution_contexts());
    tuner.set_chunk_size(chunk_size);
  }

  return internal::preduce(nullptr, first, last, identity,
                 std::forward<Reduce>(reduce),
                 std::forward<Join>(join), tuner);
}

template<typename T, class InputIterator, typename Reduce, typename Join>
symphony::task_ptr<T> preduce_async(InputIterator first,
          InputIterator last,
          const T& identity,
          Reduce&& reduce,
          Join&& join,
          symphony::pattern::tuner tuner = symphony::pattern::tuner())
{
  if (!tuner.is_chunk_set()) {
    tuner.set_chunk_size(symphony::internal::static_chunk_size<size_t>(
                         static_cast<size_t>(last - first),
                         symphony::internal::num_execution_contexts())
                      );
  }
  return symphony::internal::preduce_async(
      std::forward<Reduce>(reduce),
      std::forward<Join>(join),
      first,
      last,
      identity,
      tuner);
}

template<typename T, typename Container, typename Join>
T preduce(Container& c, const T& identity, Join&& join,
          symphony::pattern::tuner tuner = symphony::pattern::tuner())
{
  if (!tuner.is_chunk_set()){
    tuner.set_chunk_size(symphony::internal::static_chunk_size<size_t>
                         (c.size(), symphony::internal::num_execution_contexts()));
  }

  return internal::preduce
    (nullptr, c, identity, std::forward<Join>(join), tuner);
}

template<typename T, typename Container, typename Join>
symphony::task_ptr<T> preduce_async
        ( Container& c,
          const T& identity,
          Join&& join,
          symphony::pattern::tuner tuner = symphony::pattern::tuner()
        )
{
  if (!tuner.is_chunk_set()){
    tuner.set_chunk_size(symphony::internal::static_chunk_size<size_t>
                       (c.size(), symphony::internal::num_execution_contexts()));
  }

  return symphony::internal::preduce_async(std::forward<Join>(join), c, identity, tuner);
}

template<typename T, typename Iterator, typename Join>
T preduce(Iterator first, Iterator last, const T& identity, Join&& join,
          symphony::pattern::tuner tuner = symphony::pattern::tuner())
{
  if (!tuner.is_chunk_set()) {
    tuner.set_chunk_size(symphony::internal::static_chunk_size<size_t>(
                         static_cast<size_t>(last - first),
                         symphony::internal::num_execution_contexts())
                        );
  }

  return internal::preduce
    (nullptr, first, last, identity, std::forward<Join>(join), tuner);
}

template<typename T, typename Iterator, typename Join>
symphony::task_ptr<T> preduce_async
  (Iterator first, Iterator last, const T& identity, Join&& join,
   symphony::pattern::tuner tuner = symphony::pattern::tuner())
{
  if (!tuner.is_chunk_set()) {
    tuner.set_chunk_size(symphony::internal::static_chunk_size<size_t>(
                         static_cast<size_t>(last - first),
                         symphony::internal::num_execution_contexts())
                        );
  }

  return symphony::internal::preduce_async(std::forward<Join>(join), first, last, identity, tuner);
}

template<typename T, typename Reduce, typename Join, typename ...Args>
symphony::task_ptr<T> create_task(const symphony::pattern::preducer<Reduce, Join>& pr, Args&&...args)
{
  return symphony::internal::preduce_async(pr._reduce, pr._join, args...);
}

template<typename T, typename Reduce, typename Join, typename ...Args>
symphony::task_ptr<T> launch(const symphony::pattern::preducer<Reduce, Join>& pr, Args&&...args)
{
  auto t = symphony::create_task<T>(pr, args...);
  t->launch();
  return t;
}

};
