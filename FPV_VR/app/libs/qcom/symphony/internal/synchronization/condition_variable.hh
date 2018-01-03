// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <symphony/internal/synchronization/cvstatus.hh>
#include <symphony/mutex.hh>

namespace symphony{

namespace internal{

class condition_variable {
private:
  futex _futex;

public:
  static const size_t SPIN_THRESHOLD = 1000;

  condition_variable() :
    _futex() { }

  ~condition_variable() { }

  SYMPHONY_DELETE_METHOD(condition_variable(condition_variable&));
  SYMPHONY_DELETE_METHOD(condition_variable(condition_variable&&));
  SYMPHONY_DELETE_METHOD(condition_variable& operator=(condition_variable const&));
  SYMPHONY_DELETE_METHOD(condition_variable& operator=(condition_variable&&));

  void wakeup() {
    notify_all();
  }

  void notify_one() {
    _futex.wakeup(1);
  }

  void notify_all() {
    _futex.wakeup(0);
  }

  void wait( std::unique_lock<symphony::mutex>& lock) {
    lock.unlock();
    _futex.wait();
    lock.lock();
  }

  template< class Predicate >
  void wait( std::unique_lock<symphony::mutex>& lock, Predicate pred) {
    while(!pred()){
      wait(lock);
    }
  }

  template< class Rep, class Period >
  symphony::cv_status
  wait_for( std::unique_lock<symphony::mutex>& lock,
            const std::chrono::duration<Rep,Period>& rel_time) {
    auto end_time = std::chrono::high_resolution_clock::now() + rel_time;
    return wait_until(lock,end_time);
  }

  template<class Rep, class Period, class Predicate>
  bool wait_for( std::unique_lock<symphony::mutex>& lock,
                 const std::chrono::duration<Rep, Period>& rel_time,
                 Predicate pred) {
    while(!pred()){
      if(wait_for(lock, rel_time) == std::cv_status::timeout){
        return pred();
      }
    }
    return true;
  }

  template<class Clock, class Duration>
  symphony::cv_status
  wait_until( std::unique_lock<symphony::mutex>& lock,
              const std::chrono::time_point<Clock, Duration>& timeout_time) {
    lock.unlock();

    std::atomic<int>* state = _futex.timed_wait();

    while(std::chrono::high_resolution_clock::now() < timeout_time){
      if(*state == 1){
        break;
      }
      yield();
    }

    lock.lock();

    int prev_state = state->exchange(1);
    if(prev_state == 0){

      return std::cv_status::timeout;
    }

    delete state;
    return std::cv_status::no_timeout;
  }

  template<class Clock, class Duration, class Predicate>
  bool wait_until(std::unique_lock<symphony::mutex>& lock,
                  const std::chrono::time_point<Clock, Duration>&
                  timeout_time,
                  Predicate pred) {
    while(!pred()){
      if(wait_until(lock, timeout_time) == std::cv_status::timeout){
        return pred();
      }
    }
    return true;
  }
};

template<typename Lock>
class condition_variable_any {

private:
  futex _futex;

public:
  static const size_t SPIN_THRESHOLD = 1000;

  condition_variable_any() :
    _futex() {}

  ~condition_variable_any() {}

  SYMPHONY_DELETE_METHOD(condition_variable_any(condition_variable_any&));
  SYMPHONY_DELETE_METHOD(condition_variable_any(condition_variable_any&&));
  SYMPHONY_DELETE_METHOD(condition_variable_any& operator=
                     (condition_variable_any const&));
  SYMPHONY_DELETE_METHOD(condition_variable_any& operator=
                     (condition_variable_any&&));

  void wakeup() {
    notify_all();
  }

  void notify_one() {
    _futex.wakeup(1);
  }

  void notify_all() {
    _futex.wakeup(0);
  }

  void wait( Lock& lock) {
    lock.unlock();

    _futex.wait();

    lock.lock();
  }

  template< class Predicate >
  void wait( Lock& lock, Predicate pred) {
    while(!pred()){
      wait(lock);
    }
  }

  template< class Rep, class Period >
  symphony::cv_status
  wait_for( Lock& lock,
            const std::chrono::duration<Rep,Period>& rel_time) {
    auto end_time = std::chrono::high_resolution_clock::now() + rel_time;
    return wait_until(lock,end_time);
  }

  template<class Rep, class Period, class Predicate>
  bool wait_for( Lock& lock,
                 const std::chrono::duration<Rep, Period>& rel_time,
                 Predicate pred) {
    while(!pred()){
      if(wait_for(lock, rel_time) == std::cv_status::timeout){
        return pred();
      }
    }
    return true;
  }

  template<class Clock, class Duration>
  symphony::cv_status
  wait_until( Lock& lock,
              const std::chrono::time_point<Clock, Duration>& timeout_time) {
    lock.unlock();

    std::atomic<int>* state = _futex.timed_wait();

    while(std::chrono::high_resolution_clock::now() < timeout_time){
      if(*state == 1){
        break;
      }
      yield();
    }

    lock.lock();

    int prev_state = state->exchange(1);
    if(prev_state == 0){

      return std::cv_status::timeout;
    }

    delete state;
    return std::cv_status::no_timeout;
  }

  template<class Clock, class Duration, class Predicate>
  bool wait_until( Lock& lock,
                   const std::chrono::
                   time_point<Clock, Duration>& timeout_time,
                   Predicate pred) {
    while(!pred()){
      if(wait_until(lock, timeout_time) == std::cv_status::timeout){
        return pred();
      }
    }
    return true;
  }
};

};
};
