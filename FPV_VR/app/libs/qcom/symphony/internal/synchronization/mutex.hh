// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <thread>

#include <symphony/internal/synchronization/blocked_task.hh>
#include <symphony/internal/synchronization/mutex_cv.hh>
#include <symphony/internal/task/task.hh>

namespace symphony{

namespace internal{

class tts_spin_lock{
private:
  std::atomic<bool> _held;

public:
  tts_spin_lock() : _held(false) {}

  SYMPHONY_DELETE_METHOD(tts_spin_lock(tts_spin_lock &));
  SYMPHONY_DELETE_METHOD(tts_spin_lock(tts_spin_lock &&));
  SYMPHONY_DELETE_METHOD(tts_spin_lock& operator=(tts_spin_lock const &));
  SYMPHONY_DELETE_METHOD(tts_spin_lock& operator=(tts_spin_lock&&));

  void lock();
  bool try_lock();
  void unlock();
};

class futex_locked_queue{
private:
  tts_spin_lock _lock;
  std::queue<blocked_task*> _queue;

public:

  futex_locked_queue() : _lock(), _queue(){
  }

  void enqueue(blocked_task * task)
  {
    _lock.lock();
    _queue.push(task);
    _lock.unlock();
  }

  blocked_task* dequeue()
  {
    blocked_task* result = nullptr;
    _lock.lock();
    if(_queue.empty() == false){
      result = _queue.front();
      _queue.pop();
    }
    _lock.unlock();
    return result;
  }

  blocked_task* unlocked_dequeue()
  {
    blocked_task* result = nullptr;
    if(_queue.empty() == false){
      result = _queue.front();
      _queue.pop();
    }
    return result;
  }

  tts_spin_lock& get_lock(){
    return _lock;
  }
};

class futex{
private:
  futex_locked_queue _blocked_tasks;
  std::atomic<size_t> _waiters;
  std::atomic<size_t> _awake;

public:
  futex() : _blocked_tasks(), _waiters(0), _awake(0) {
  }

  SYMPHONY_DELETE_METHOD(futex(futex&));
  SYMPHONY_DELETE_METHOD(futex(futex&&));
  SYMPHONY_DELETE_METHOD(futex& operator=(futex const&));
  SYMPHONY_DELETE_METHOD(futex& operator=(futex&&));

  void wait(std::atomic<int>* check_value_ptr = nullptr, int check_value = 0);

  std::atomic<int>* timed_wait(std::atomic<int>* check_value_ptr = nullptr,
                               int check_value = 0);

  size_t wakeup(size_t num_tasks);

private:

  size_t wakeup_one(bool unlocked = false);
};

class mutex{
private:
  std::atomic<int> _state;
  futex _futex;

public:

  static const int SPIN_THRESHOLD = 10;

  mutex() : _state(0), _futex()
  {}

  SYMPHONY_DELETE_METHOD(mutex(mutex&));
  SYMPHONY_DELETE_METHOD(mutex(mutex&&));
  SYMPHONY_DELETE_METHOD(mutex& operator=(mutex const&));
  SYMPHONY_DELETE_METHOD(mutex& operator=(mutex&&));

  void lock();
  bool try_lock();
  void unlock();
};

template<class Lock>
class recursive_lock{
private:
  Lock _lock;
  std::thread::id _current_holder;
  size_t _permits;

  std::thread::id get_id(){
    return std::this_thread::get_id();
  }

  bool is_owner(){
    std::thread::id task_id = get_id();
    return _current_holder == task_id;
  }

  void set_owner(){
    _current_holder = get_id();
    _permits = 1;
  }

public:

  recursive_lock() : _lock(), _current_holder(0), _permits(0){
  }

  SYMPHONY_DELETE_METHOD(recursive_lock(recursive_lock&));
  SYMPHONY_DELETE_METHOD(recursive_lock(recursive_lock&&));
  SYMPHONY_DELETE_METHOD(recursive_lock& operator=(recursive_lock const&));
  SYMPHONY_DELETE_METHOD(recursive_lock& operator=(recursive_lock&&));

  void lock(){
    if(is_owner()){
      ++_permits;
    }else{
      _lock.lock();
      set_owner();
    }
  }

  bool try_lock(){
    if(is_owner()){
      ++_permits;
      return true;
    }else{
      bool success = _lock.try_lock();
      if(success){
        set_owner();
      }
      return success;
    }
  }

  void unlock(){
    --_permits;
    if(_permits == 0){
      _current_holder = std::thread::id(0);
      _lock.unlock();
    }
  }
};

template<class Lock>
class timed_lock{
private:
  Lock _lock;

public:
  static const size_t SPIN_THRESHOLD = 1000;

  timed_lock() : _lock(){
  }

  SYMPHONY_DELETE_METHOD(timed_lock(timed_lock&));
  SYMPHONY_DELETE_METHOD(timed_lock(timed_lock&&));
  SYMPHONY_DELETE_METHOD(timed_lock& operator=(timed_lock const&));
  SYMPHONY_DELETE_METHOD(timed_lock& operator=(timed_lock&&));

  void lock()
  {
    _lock.lock();
  }

  bool try_lock()
  {
    return _lock.try_lock();
  }

  template<class Rep, class Period>
  bool try_lock_for( const std::chrono::duration<Rep,Period>& timeout_duration)
  {
    auto end_time = std::chrono::high_resolution_clock::now() +
      timeout_duration;
    size_t local_spins = 0;

    while(_lock.try_lock() == false){
      if(end_time >= std::chrono::high_resolution_clock::now()){
        return false;
      }

      if(local_spins > SPIN_THRESHOLD){
        yield();
        local_spins = 0;
      }
    }

    return true;
  }

  template<class Clock, class Duration>
  bool try_lock_until(const std::chrono::
                      time_point<Clock,Duration>& timeout_time)
  {
    size_t local_spins = 0;

    while(_lock.try_lock() == false){
      if(timeout_time >= std::chrono::high_resolution_clock::now()){
        return false;
      }

      if(local_spins > SPIN_THRESHOLD){
        yield();
        local_spins = 0;
      }
    }

    return true;
  }
};

};

};
