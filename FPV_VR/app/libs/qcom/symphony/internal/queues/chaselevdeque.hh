// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <mutex>

#include <symphony/internal/atomic/atomicops.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/memorder.hh>

#define SYMPHONY_CLD_SERIALIZE 0

#define SYMPHONY_CLD_FORCE_SEQ_CST 0

#if SYMPHONY_CLD_SERIALIZE
#define SYMPHONY_CLD_LOCK std::lock_guard<std::mutex> lock(_serializing_mutex)
#else
#define SYMPHONY_CLD_LOCK do { } while(0)
#endif

#if SYMPHONY_CLD_FORCE_SEQ_CST
#define SYMPHONY_CLD_MO(x) symphony::mem_order_seq_cst
#else
#define SYMPHONY_CLD_MO(x) x
#endif

template <typename T>
class chase_lev_array {
public:
  explicit chase_lev_array(unsigned logsz = 12) :
    _logsz(logsz),
    _arr(new std::atomic<T>[size_t(1) << logsz]),
    _next(nullptr){}

  ~chase_lev_array() { delete[] _arr; }

  explicit chase_lev_array(T&) { }

  size_t size() const {
    return size_t(1) << _logsz;
  }

  T get(size_t i,
        symphony::mem_order mo = SYMPHONY_CLD_MO(symphony::mem_order_relaxed)) {
    return _arr[i & (size() - 1)].load(mo);
  }

  void put(size_t i, T x,
           symphony::mem_order mo = SYMPHONY_CLD_MO(symphony::mem_order_relaxed)) {
    _arr[i & (size() - 1)].store(x, mo);
  }

  chase_lev_array<T> *resize(size_t bot, size_t top) {
    auto a = new chase_lev_array<T>(_logsz + 1);

    SYMPHONY_INTERNAL_ASSERT(top <= bot, "oops");

    for (size_t i = top; i < bot; ++i)
      a->put(i, get(i));
    return a;
  }

  void set_next(chase_lev_array<T>* next){_next = next;}
  chase_lev_array<T>* get_next(){ return _next;}
private:
  unsigned _logsz;
  std::atomic<T>* _arr;

  chase_lev_array<T>* _next;

  chase_lev_array<T> & operator=(const chase_lev_array<T>&);
  chase_lev_array<T>(const chase_lev_array<T>&);
};

template <typename T, bool Lepop = true>
class chase_lev_deque {
public:

  static constexpr size_t s_abort = ~size_t(0);

  explicit chase_lev_deque(unsigned logsz = 12):
    _top(1),
    _bottom(1),
    _cl_array(new chase_lev_array<T>(logsz)),
    _serializing_mutex(),
    _original_head(_cl_array){}

  virtual ~chase_lev_deque() {

    SYMPHONY_INTERNAL_ASSERT(_original_head != nullptr, "Error. Head is nullptr");
    while(_original_head != nullptr){
      auto cur = _original_head;
      _original_head = cur->get_next();
      delete cur;
    }
  }

  SYMPHONY_DELETE_METHOD(chase_lev_deque(chase_lev_deque const&));
  SYMPHONY_DELETE_METHOD(chase_lev_deque(chase_lev_deque &&));
  SYMPHONY_DELETE_METHOD(chase_lev_deque& operator=(chase_lev_deque const&));
  SYMPHONY_DELETE_METHOD(chase_lev_deque& operator=(chase_lev_deque &&));

  size_t take(T& x) {
    SYMPHONY_CLD_LOCK;

    size_t b = _bottom.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

    if (b == 0)
      return 0;

    b = _bottom.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    auto a = _cl_array.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

    --b;

    _bottom.store(b, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    symphony::internal::symphony_atomic_thread_fence(
        SYMPHONY_CLD_MO(symphony::mem_order_seq_cst));

    size_t t = _top.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

    if (b < t) {

      if (Lepop)
        _bottom.store(b + 1, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
      else
        _bottom.store(t, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
      return 0;
    }

    auto current_x = x;
    x = a->get(b);
    if (b > t) {

      return b - t;
    }

    size_t sz = 1;

    if (!_top.compare_exchange_strong(
            t, t + 1,
            SYMPHONY_CLD_MO(symphony::mem_order_seq_cst),
            SYMPHONY_CLD_MO(symphony::mem_order_relaxed))) {

      x = current_x;
      sz = 0;
    }

    if (Lepop)
      _bottom.store(b + 1, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    else
      _bottom.store(t + 1, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

    return sz;
  }

  size_t push(T x) {
    SYMPHONY_CLD_LOCK;
    size_t b = _bottom.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    size_t t = _top.load(SYMPHONY_CLD_MO(symphony::mem_order_acquire));
    auto a = _cl_array.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

    if (b >= t + a->size()) {

      auto old_a = a;
      a = a->resize(b, t);
      _cl_array.store(a, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));

      old_a->set_next(a);
    }
    a->put(b, x);
    symphony::internal::symphony_atomic_thread_fence(
        SYMPHONY_CLD_MO(symphony::mem_order_release));
    _bottom.store(b + 1, SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    return b - t;
  }

  size_t steal(T& x) {
    SYMPHONY_CLD_LOCK;
    size_t t = _top.load(SYMPHONY_CLD_MO(symphony::mem_order_acquire));
    symphony::internal::symphony_atomic_thread_fence(
        SYMPHONY_CLD_MO(symphony::mem_order_seq_cst));
    size_t b = _bottom.load(SYMPHONY_CLD_MO(symphony::mem_order_acquire));

    if (t >= b)
      return 0;

    auto current_x = x;
    auto a = _cl_array.load(SYMPHONY_CLD_MO(symphony::mem_order_relaxed));
    x = a->get(t);
    if (!_top.compare_exchange_strong(
            t, t + 1,
            SYMPHONY_CLD_MO(symphony::mem_order_seq_cst),
            SYMPHONY_CLD_MO(symphony::mem_order_relaxed))) {

      x = current_x;
      return s_abort;
    }
    return b - t;
  }

  size_t unsafe_size(symphony::mem_order order = symphony::mem_order_relaxed) {
    auto t = _top.load(SYMPHONY_CLD_MO(order));
    auto b = _bottom.load(SYMPHONY_CLD_MO(order));

    return b - t;
  }

private:

  std::atomic<size_t> _top;
  std::atomic<size_t> _bottom;
  std::atomic<chase_lev_array<T>*> _cl_array;
  std::mutex _serializing_mutex;

  chase_lev_array<T>* _original_head;
};
