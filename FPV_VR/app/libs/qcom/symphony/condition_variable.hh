// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/synchronization/condition_variable.hh>

namespace symphony{

template<class Lock>

struct condition_variable_any{
  typedef internal::condition_variable_any<Lock> type;
};

typedef internal::condition_variable condition_variable;

};

#ifdef ONLY_FOR_DOXYGEN

#include <mutex>

#include <symphony/mutex.hh>

namespace symphony{

class condition_variable {
public:

  constexpr condition_variable();

  condition_variable(condition_variable const&) = delete;
  condition_variable(condition_variable&&) = delete;
  condition_variable& operator=(condition_variable const&) = delete;

  void notify_one();

  void notify_all();

  void wait(std::unique_lock<symphony::mutex>& lock);

  template< class Predicate >
  void wait( std::unique_lock<symphony::mutex> & lock, Predicate pred);

  template< class Rep, class Period >
  std::cv_status wait_for( std::unique_lock<symphony::mutex> & lock,
                           const std::chrono::duration<Rep,Period> & rel_time);

  template<class Rep, class Period, class Predicate>
  bool wait_for( std::unique_lock<symphony::mutex> & lock,
                 const std::chrono::duration<Rep, Period> & rel_time,
                 Predicate pred);

  template<class Clock, class Duration>
  std::cv_status
  wait_until(std::unique_lock<symphony::mutex>& lock,
             const std::chrono::time_point<Clock, Duration>& timeout_time);

  template<class Clock, class Duration, class Predicate>
  bool wait_until(std::unique_lock<symphony::mutex>& lock,
                  const std::chrono::time_point<Clock, Duration>& timeout_time,
                  Predicate pred);
};

};

#endif
