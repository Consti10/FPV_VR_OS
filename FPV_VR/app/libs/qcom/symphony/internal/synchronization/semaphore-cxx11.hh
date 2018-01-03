// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/synchronization/cvstatus.hh>

namespace symphony {

namespace internal {

class semaphore {
public:
  explicit semaphore(bool flag = false)
    : _flag(flag),
      _mutex(),
      _cv()
  {}

  ~semaphore() {}

  SYMPHONY_DELETE_METHOD(semaphore(semaphore const&));
  SYMPHONY_DELETE_METHOD(const semaphore& operator=(semaphore const&));

  void signal() {
    reset(true);
  }

  template<typename Fn>
  void signal(Fn&& fn) {
    std::lock_guard<std::mutex> lock(_mutex);
    fn();
    _flag = true;
    _cv.notify_one();
  }

  void reset(bool flag = false) {
    std::lock_guard<std::mutex> lock(_mutex);
    _flag = flag;
    if (flag)
      _cv.notify_one();
  }

  void wait() {
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_flag)
      _cv.wait(lock);
    _flag = false;
  }

  template<class Rep, class Period>
  symphony::cv_status
  wait_for(std::chrono::duration<Rep, Period> const& rel_time) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_flag) {
      if (_cv.wait_for(lock, rel_time) == std::cv_status::timeout)
        return std::cv_status::timeout;
    }
    _flag = false;
    return std::cv_status::no_timeout;
  }

  std::mutex& mutex() {
    return _mutex;
  }

protected:
  bool _flag;
  std::mutex _mutex;
  std::condition_variable _cv;
};

class counting_semaphore {
public:
  explicit counting_semaphore(size_t count = 1)
    : _count(count),
      _mutex(),
      _cv()
  {}

  ~counting_semaphore() {}

  SYMPHONY_DELETE_METHOD(counting_semaphore(counting_semaphore const&));
  SYMPHONY_DELETE_METHOD(const counting_semaphore& operator=
                     (counting_semaphore const&));

  void signal(size_t times = 1) {
    std::lock_guard<std::mutex> lock(_mutex);
    SYMPHONY_INTERNAL_ASSERT(_count >= times, "unbalanced signals");
    size_t res = (_count -= times);
    if (res == 0) {
      _cv.notify_all();
    }
  }

  void wait() {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_count > 0)
      _cv.wait(lock);
  }

  template<class Rep, class Period>
  symphony::cv_status
  wait_for(std::chrono::duration<Rep, Period> const& rel_time) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_count > 0) {
      if (_cv.wait_for(lock, rel_time) == std::cv_status::timeout)
        return std::cv_status::timeout;
    }
    return std::cv_status::no_timeout;
  }

  void reset(size_t count = 1) {
    std::lock_guard<std::mutex> lock(_mutex);
    _count = count;
  }

protected:
  size_t _count;
  std::mutex _mutex;
  std::condition_variable _cv;
};

};

};
