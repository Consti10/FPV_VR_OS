// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <chrono>

#include <symphony/internal/synchronization/mutex.hh>

namespace symphony{

typedef internal::mutex mutex;
typedef internal::timed_lock<symphony::mutex> timed_mutex;
typedef internal::recursive_lock<symphony::mutex> recursive_mutex;
typedef internal::timed_lock<symphony::recursive_mutex> recursive_timed_mutex;

typedef internal::futex futex;

};

#ifdef ONLY_FOR_DOXYGEN

namespace symphony {

class mutex {

public:

  constexpr mutex();

  mutex(mutex const&) = delete;
  mutex(mutex&&) = delete;
  mutex& operator=(mutex const&) = delete;

  ~mutex();

  void lock();

  bool try_lock();

  void unlock();
};

class futex {

public:

  futex();

  ~futex();

  futex(futex const&) = delete;
  futex(futex&&) = delete;
  futex& operator=(futex const&) = delete;

  void wait();

  size_t wakeup(size_t num_tasks);
};

};

#endif
