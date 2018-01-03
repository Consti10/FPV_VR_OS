// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <cstdlib>

#include <symphony/internal/legacy/task.hh>
#include <symphony/internal/synchronization/mutex.hh>

namespace symphony{

namespace internal{

class sense_barrier{
private:

  std::atomic<size_t> _count;

  size_t _total;

  std::atomic<bool> _sense;

  task_shared_ptr _wait_task[2];

  size_t _delta;

public:
  static const int WAIT_TASK_SPIN_THRESHOLD = 1000;
  static const int SPIN_THRESHOLD = 10000;

  void wait();

  explicit sense_barrier(size_t count) : _count(count), _total(count),
                                         _sense(false), _wait_task(),
                                         _delta(_total / (4))
  {
    _wait_task[0] = nullptr;
    _wait_task[1] = nullptr;
  }

  SYMPHONY_DELETE_METHOD(sense_barrier(sense_barrier&));
  SYMPHONY_DELETE_METHOD(sense_barrier(sense_barrier&&));
  SYMPHONY_DELETE_METHOD(sense_barrier& operator=(sense_barrier const&));
  SYMPHONY_DELETE_METHOD(sense_barrier& operator=(sense_barrier&&));

private:

  bool volatile_read_sense(){
    return _sense.load(symphony::mem_order_relaxed);
  }

  void create_wait_task(bool local_sense);
};

};

};
