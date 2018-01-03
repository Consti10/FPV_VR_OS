// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/atomic/atomicops.hh>
#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/memorder.hh>

#define SYMPHONY_BLFQ_FORCE_SEQ_CST 0

#if SYMPHONY_BLFQ_FORCE_SEQ_CST
#define SYMPHONY_BLFQ_MO(x) symphony::mem_order_seq_cst
#else
#define SYMPHONY_BLFQ_MO(x) x
#endif

namespace symphony{

namespace internal{

namespace blfq{

struct element
{
#if SYMPHONY_HAS_ATOMIC_DMWORD
  typedef symphony_dmword_t element_t;
  element_t _safe    : 1;
  element_t _empty   : 1;
  #if SYMPHONY_SIZEOF_MWORD == 8
    element_t _idx     : 62;
    element_t _val     : 64;
  #elif SYMPHONY_SIZEOF_MWORD == 4
    element_t _idx     : 30;
    element_t _val     : 32;
  #else
    #error "unknown SYMPHONY_SIZEOF_MWORD"
  #endif
#else
  #error "The Bounded Lock-Free Queue requires Double Machine Word atomics"
#endif

    element(element_t safe, element_t empty, element_t idx, element_t val) :
    _safe(safe),
    _empty(empty),
    _idx(idx),
    _val(val) {}

    element() SYMPHONY_NOEXCEPT: _safe(), _empty(), _idx(), _val() {}
};

class bounded_lfqueue{

private:

  static constexpr size_t s_starving_limit = 10000;

  inline void close_node(){

#if SYMPHONY_SIZEOF_MWORD == 8
    _tail.fetch_or(size_t(1) << 63, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
#elif SYMPHONY_SIZEOF_MWORD == 4
    _tail.fetch_or(size_t(1) << 31, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
#else
    #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
  }

  inline size_t extract_index_from_tail(size_t tail){

    return((tail << 1) >> 1);
  }

  inline bool is_node_closed(size_t tail){

#if SYMPHONY_SIZEOF_MWORD == 8
    return((tail & (size_t(1) << 63)) != 0);
#elif SYMPHONY_SIZEOF_MWORD == 4
    return((tail & (size_t(1) << 31)) != 0);
#else
    #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
  }

  void fix_state(bool is_unbounded_queue = true){
    while(true){
      auto t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
      auto h = _head.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

      if(h <= t){
        if(is_unbounded_queue){
          return;
        }
        else{

          if(is_node_closed(t)){
            open_node();
          }
          return;
        }
      }
      else{

        if(_tail.compare_exchange_strong(t,h, SYMPHONY_BLFQ_MO(symphony::mem_order_acq_rel), symphony::mem_order_relaxed)){
          return;
        }
      }
    }
  }

  void open_node(){
    while(true){
      auto t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

      if(!is_node_closed(t)){
        return;
      }

#if SYMPHONY_SIZEOF_MWORD == 8
      auto temp = t&(~(size_t(1) << 63));
      if(_tail.compare_exchange_strong(t,temp, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){
        return;
      }

#elif SYMPHONY_SIZEOF_MWORD == 4
      auto temp = t&(~(size_t(1) << 31));
      if(_tail.compare_exchange_strong(t,temp, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){
        return;
      }
#else
      #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
    }
  }

  inline bool ok_to_increment(size_t current){
#if SYMPHONY_SIZEOF_MWORD == 8
    if((current + 1) < (size_t(1) << 62)){
      return true;
    }
#elif SYMPHONY_SIZEOF_MWORD == 4
    if((current + 1) < (size_t(1) << 30)){
      return true;
    }
#else
    #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
    return false;
  }

public:

  explicit bounded_lfqueue(size_t logsz):
    _head(0),
    _tail(0),
    _logsz(logsz),
    _max_array_size(size_t(1) << logsz),
    _array( nullptr){

#if SYMPHONY_SIZEOF_MWORD == 8
      SYMPHONY_INTERNAL_ASSERT(logsz <= 62, "On 64 bit systems the largest size of the blfq array allowed is 2^62");
#elif SYMPHONY_SIZEOF_MWORD == 4
      SYMPHONY_INTERNAL_ASSERT(logsz <= 30, "On 32 bit systems the largest size of the blfq array allowed is 2^30");
#else
      #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif

    _array = new std::atomic<element>[_max_array_size];

    for (size_t i = 0; i < _max_array_size; i++) {
      element a;
      a._safe = 1;
      a._empty = 1;
      a._idx  = i;
      a._val  = 0;

      *(reinterpret_cast<element*>(&_array[i])) = a;
    }
    symphony_atomic_thread_fence(SYMPHONY_BLFQ_MO(symphony::mem_order_seq_cst));

  }

  ~bounded_lfqueue(){
    delete [] _array;
  }

  SYMPHONY_DELETE_METHOD(bounded_lfqueue());

  SYMPHONY_DELETE_METHOD(bounded_lfqueue(bounded_lfqueue const&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue(bounded_lfqueue &&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue& operator=(bounded_lfqueue const&));
  SYMPHONY_DELETE_METHOD(bounded_lfqueue& operator=(bounded_lfqueue &&));

  void lock_node() {
    close_node();

    size_t tail = extract_index_from_tail(_tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed)));
    if (tail > _max_array_size) {

      _head.store((tail - _max_array_size), SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
    }
  }

  size_t take(size_t * result, bool is_unbounded_queue){
    size_t num_iter = 0;
    while(true){

      size_t h = 0;
#if SYMPHONY_SIZEOF_MWORD == 8
      h = _head.fetch_add(1,SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
#elif SYMPHONY_SIZEOF_MWORD == 4
      while(true){
        h = _head.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
        if(ok_to_increment(h)){
          if(_head.compare_exchange_strong(h,h+1,SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed)))
            break;
        }
        else{

          return 0;
        }
      }
#else
      #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
      while(true){

        auto current_element = _array[h & (_max_array_size - 1)].load(SYMPHONY_BLFQ_MO(symphony::mem_order_acquire));

        if(current_element._idx > h){
          break;
        }

        if(current_element._empty == 0){
          if(current_element._idx == h){
            if(_array[h & (_max_array_size - 1)].compare_exchange_strong(current_element,
                 element(current_element._safe, 1, (h + _max_array_size), 0),
                 SYMPHONY_BLFQ_MO(symphony::mem_order_release),SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

              *result = current_element._val;

              auto t = extract_index_from_tail(_tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed)));

              SYMPHONY_INTERNAL_ASSERT(t > h, "Error. head has to be strictly less than tail as the pop was successful");

              if(!is_unbounded_queue){
                open_node();
              }

              return (t - h);
            }
          }
          else{

            if(_array[h & (_max_array_size - 1)].compare_exchange_strong(current_element,
                 element(0, current_element._empty, current_element._idx, current_element._val),
                 SYMPHONY_BLFQ_MO(symphony::mem_order_release), SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){
              break;
            }
          }
        }

        else{

          auto t = extract_index_from_tail(_tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed)));
          if(t >= (h+1)){

            num_iter++;
          }
          else{

            num_iter = s_starving_limit;
          }

          if(num_iter >= s_starving_limit){
            if(_array[h & (_max_array_size - 1)].compare_exchange_strong(current_element,
                 element(current_element._safe, current_element._empty, (h + _max_array_size), 0),
                 SYMPHONY_BLFQ_MO(symphony::mem_order_release), SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){
              break;
            }
          }
          continue;
        }
      }

      auto t = extract_index_from_tail(_tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed)));
      if(t <= (h + 1)){

        fix_state(is_unbounded_queue);
        return 0;
      }

    }
    return 0;
  }

  size_t unbounded_put(size_t value){
    while(true){

      size_t t = 0;
      size_t h = 0;

      size_t num_retries = 0;
#if SYMPHONY_SIZEOF_MWORD == 8
      t = _tail.fetch_add(1, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

      if(is_node_closed(t)){
        return 0;
      }

      SYMPHONY_INTERNAL_ASSERT(((t & (size_t(1) << 62)) == 0), "tail should be less than 2^62");

#elif SYMPHONY_SIZEOF_MWORD == 4

      while(true){
        t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

        if(is_node_closed(t)){
          return 0;
        }

        if(ok_to_increment(t)){

          if(_tail.compare_exchange_strong(t, t+1, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            break;
          }
        }
        else{

          close_node();
          return 0;
        }
      }
#else
    #error "unknown SYMPHONY_SIZEOF_MWORD"
#endif
      auto current_element = _array[t & (_max_array_size - 1)].load(SYMPHONY_BLFQ_MO(symphony::mem_order_acquire));

      h = _head.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

      if(current_element._empty != 0){
        if((current_element._idx <= t) && ((current_element._safe != 0) || (h <= t))){
          if(_array[t & (_max_array_size - 1)].compare_exchange_strong(
               current_element, element(1,0,t,value), SYMPHONY_BLFQ_MO(symphony::mem_order_release),
               SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            if(h <= t){
              return ((t + 1) - h);
            }
            else{

              return 1;
            }
          }
          else{

            num_retries++;
          }
        }
      }

      if((((t - h) >= _max_array_size) && (t > h)) || (num_retries > s_starving_limit)){
        close_node();
        return 0;
      }
    }
  }

  size_t bounded_put(size_t value){
    while(true){
     size_t t = 0;

      while(true){
        t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

        if(is_node_closed(t)){
          return 0;
        }

        if(ok_to_increment(t)){

          if(_tail.compare_exchange_strong(t, t+1, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            break;
          }
        }
        else{

          close_node();
          return 0;
        }
      }

      auto current_element = _array[t & (_max_array_size - 1)].load(SYMPHONY_BLFQ_MO(symphony::mem_order_acquire));

      size_t h = _head.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

      if(current_element._empty != 0){
        if((current_element._idx <= t) && ((current_element._safe != 0) || (h <= t))){
          if(_array[t & (_max_array_size - 1)].compare_exchange_strong(
               current_element, element(1,0,t,value), SYMPHONY_BLFQ_MO(symphony::mem_order_release),
               SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            if(h <= t){
              return (t+1)-h;
            }
            else{

              return 1;
            }
          }
        }
      }

      if((((t - h) >= _max_array_size) && (t > h))){
        close_node();
        return 0;
      }
    }
  }

  size_t overwrite_put(size_t value, size_t * result){
    while(true){

      size_t t;
      while(true){
        t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));

        if(is_node_closed(t)){
          return 0;
        }

        if(ok_to_increment(t)){

          if(_tail.compare_exchange_strong(t, t+1, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            break;
          }
        }
      }

      auto current_element = _array[t & (_max_array_size - 1)].load(SYMPHONY_BLFQ_MO(symphony::mem_order_acquire));

      if(current_element._safe != 0){
        if(_array[t & (_max_array_size - 1)].compare_exchange_strong(
             current_element, element(1,0,t,value), SYMPHONY_BLFQ_MO(symphony::mem_order_release),
             SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

          *result = current_element._val;
          if(current_element._empty != 0){

            return _max_array_size;
          }
          else{
            return _max_array_size + 1;
          }
        }
        else{

          continue;
        }
      }
      else{

        return 0;
      }
    }
  }

  size_t consume(size_t * result){
    while(true){
      auto head  = _head.fetch_add(1, SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
      while(true){
        auto current_element = _array[head & (_max_array_size - 1)].load(SYMPHONY_BLFQ_MO(symphony::mem_order_acquire));
        if(current_element._safe != 0){
          if(_array[head & (_max_array_size - 1)].compare_exchange_strong(
               current_element, element(0,1, head + _max_array_size, 0), SYMPHONY_BLFQ_MO(symphony::mem_order_release),
               SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed))){

            if (current_element._empty == 0) {
              *(result) = current_element._val;
              return 1;
            }
            else{

              break;
            }
          }
        }
        else{

          SYMPHONY_INTERNAL_ASSERT(current_element._empty == 1, "Error. Node is unsafe but not empty");
          return 0;
        }
      }
    }
  }

  size_t get_size() const{
    auto t = _tail.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
    auto h = _head.load(SYMPHONY_BLFQ_MO(symphony::mem_order_relaxed));
    return (t - h);
  }

  size_t get_log_array_size() const{ return _logsz;}

  size_t get_max_array_size() const{ return _max_array_size;}

private:

  std::atomic<size_t> _head;

  std::atomic<size_t> _tail;

  size_t const _logsz;

  size_t const _max_array_size;

  std::atomic<element>*  _array;

};

template<typename T, bool FITS_IN_SIZE_T>
class blfq_size_t
{

};

template<typename T>
class blfq_size_t<T,false>
{
static_assert(sizeof(T*) <= sizeof(size_t), "Error. Address has size greater than sizeof(size_t)");

public:
  explicit blfq_size_t(size_t log_size = 12) : _blfq(log_size) { }

  SYMPHONY_DELETE_METHOD(blfq_size_t(blfq_size_t const&));
  SYMPHONY_DELETE_METHOD(blfq_size_t(blfq_size_t &&));
  SYMPHONY_DELETE_METHOD(blfq_size_t& operator=(blfq_size_t const&));
  SYMPHONY_DELETE_METHOD(blfq_size_t& operator=(blfq_size_t &&));

  size_t push(T const& value, bool is_unbounded_queue)
  {

    T * ptr = new T(value);
    if(is_unbounded_queue)
      return(_blfq.unbounded_put(reinterpret_cast<size_t>(ptr)));
    else
      return(_blfq.bounded_put(reinterpret_cast<size_t>(ptr)));
  }

  SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing")
  size_t produce(T const& value, T& result) {
    T * ptr = new T(value);
    T *ret_ptr;
    size_t res = _blfq.overwrite_put(reinterpret_cast<size_t>(ptr), reinterpret_cast<size_t*>(&ret_ptr));
    if(res != 0){
      if(ret_ptr != nullptr){
        result = *ret_ptr;
        delete ret_ptr;
      }
    }
    return res;
  }

  size_t pop(T& result, bool is_unbounded_queue) {
    T *ptr;
    size_t res = _blfq.take(reinterpret_cast<size_t*>(&ptr), is_unbounded_queue);
    if (res != 0){
      result = *ptr;

      delete ptr;
    }
    return res;
  }

  size_t consume(T& result) {
    T *ptr;
    size_t res = _blfq.consume(reinterpret_cast<size_t*>(&ptr));
    if (res != 0){
      result = *ptr;

      delete ptr;
    }
    return res;
  }
SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing")

  void close() {_blfq.lock_node();}

  size_t size() const{ return _blfq.get_size(); }

  size_t get_log_array_size() const{ return _blfq.get_log_array_size(); }

  size_t get_max_array_size() const{ return _blfq.get_max_array_size(); }

private:
  bounded_lfqueue _blfq;

};

template <typename T>
struct blfq_convert
{
  static_assert(sizeof(T) <= sizeof(size_t), "Error. sizeof(T) > sizeof(size_t)");

SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing")
  static size_t cast_from(T v) {
    size_t u;
    *(reinterpret_cast<T*>(&u)) = v;
    return u;
  }

  static T cast_to(size_t v) {
    return(*(reinterpret_cast<T*>(&v)));
  }
SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing")
};

template <typename T>
struct blfq_convert<T*>
{
  static_assert(sizeof(T*) <= sizeof(size_t), "Error. sizeof(T*) > sizeof(size_t)");

  static size_t cast_from(T* v) { return reinterpret_cast<size_t>(v); }
  static T* cast_to(size_t v) { return reinterpret_cast<T*>(v); }
};

template<typename T>
class blfq_size_t<T, true>
{
public:
  typedef T value_type;
  blfq_size_t<T, true>(size_t log_size = 12) : _blfq(log_size) {}

  SYMPHONY_DELETE_METHOD(blfq_size_t(blfq_size_t const&));
  SYMPHONY_DELETE_METHOD(blfq_size_t(blfq_size_t &&));
  SYMPHONY_DELETE_METHOD(blfq_size_t& operator=(blfq_size_t const&));
  SYMPHONY_DELETE_METHOD(blfq_size_t& operator=(blfq_size_t &&));

  size_t push(value_type const& value, bool is_unbounded_queue) {
    if(is_unbounded_queue)
      return (_blfq.unbounded_put(blfq_convert<T>::cast_from(value)));
    else
      return (_blfq.bounded_put(blfq_convert<T>::cast_from(value)));
  }

  size_t produce(value_type const& value, value_type& result) {
    size_t tmp;
    size_t res =_blfq.overwrite_put(blfq_convert<T>::cast_from(value), &tmp);
    if(res != 0){
      result = blfq_convert<T>::cast_to(tmp);
    }
    return (res);
  }

  size_t pop(value_type& result, bool is_unbounded_queue) {
    size_t tmp;
    size_t res = _blfq.take(&tmp, is_unbounded_queue);
    if (res != 0){
      result = blfq_convert<T>::cast_to(tmp);
    }
    return res;
  }

  size_t consume(value_type& result) {
    size_t tmp;
    size_t res = _blfq.consume(&tmp);
    if (res != 0){
      result = blfq_convert<T>::cast_to(tmp);
    }
    return res;
  }

  void close() {_blfq.lock_node();}

  size_t size() const{ return _blfq.get_size();}

  size_t get_log_array_size() const{ return _blfq.get_log_array_size(); }

  size_t get_max_array_size() const{ return _blfq.get_max_array_size(); }

private:
  bounded_lfqueue _blfq;

};

};

};

};
