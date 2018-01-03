// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <string>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/memalloc/defaulttaskallocator.hh>
#include <symphony/internal/memalloc/linkedlistpool.hh>
#include <symphony/internal/util/memorder.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class thread_local_fixed_pool_set: public pool {

  class spin_lock {

  private:
    std::atomic<bool> _held;

  public:

    spin_lock() : _held(false) {}

    SYMPHONY_DELETE_METHOD(spin_lock(spin_lock &));
    SYMPHONY_DELETE_METHOD(spin_lock(spin_lock &&));
    SYMPHONY_DELETE_METHOD(spin_lock& operator=(spin_lock const &));
    SYMPHONY_DELETE_METHOD(spin_lock& operator=(spin_lock&&));

    void lock() {
      bool acquired = false;

      while(acquired == false) {

        while(_held.load(symphony::mem_order_relaxed) == true){}

        bool old_value = false;
        acquired = _held.compare_exchange_strong(old_value, true,  symphony::mem_order_acquire);
      }
    }

    void unlock(){
      _held.store(false, symphony::mem_order_release);
    }
  };

    static constexpr size_t s_alignment_padding_bytes = 128;

public:

  thread_local_fixed_pool_set(size_t threadid, size_t pool_size, size_t object_size):
    _merged_local_free_list(nullptr),
    _allocs(0),
    _local_deallocs(0),
    _tid(threadid),
    _first_pool(pool_size, object_size, this),
    _active_pool(nullptr),
    _last_pool(&_first_pool),
    _lock(),
    _external_deallocs(0),
    _list_swaps(0),
    _merged_external_free_list(nullptr) {
    set_active_pool(&_first_pool);
    SYMPHONY_UNUSED(_padding);
  }

  ~thread_local_fixed_pool_set() {

    linkedlist_pool* curr_pool = _first_pool.get_next_pool();
    linkedlist_pool* next_pool;
    while(curr_pool != nullptr) {
      next_pool = curr_pool->get_next_pool();
      delete curr_pool;
      curr_pool = next_pool;
    }
  }

  void remove_pool(linkedlist_pool* old_pool) {
    SYMPHONY_INTERNAL_ASSERT(old_pool != &_first_pool,
                         "The first pool cannot be removed!");
    linkedlist_pool* prev_pool = old_pool->get_prev_pool();
    linkedlist_pool* next_pool = old_pool->get_prev_pool();
    if (prev_pool != nullptr) {
      prev_pool->set_next_pool(next_pool);
    }
    if (next_pool != nullptr) {
      next_pool->set_prev_pool(prev_pool);
    }
  }

  void add_pool(linkedlist_pool* new_pool) {
    SYMPHONY_INTERNAL_ASSERT(new_pool->_next_pool == nullptr, "New pool not initialized properly!");
    _last_pool->set_next_pool(new_pool);
    if (new_pool != nullptr) {
      new_pool->set_prev_pool(_last_pool);
      _last_pool = new_pool;
    }
  }

  void set_active_pool(linkedlist_pool* active_pool) {
    _active_pool = active_pool;
    _merged_local_free_list = _active_pool->get_first_entry();
  }

  virtual bool deallocate_object(char* object) {

    auto ti = tls::get();
    auto threadid = ti->allocator_id();
    return deallocate_object_impl(object, threadid);
  }

  virtual bool deallocate_object(char* object, size_t threadid) {
    return deallocate_object_impl(object, threadid);
  }

  char* allocate_object() {

    _allocs++;
    char* object = linkedlist_pool::allocate_raw(_merged_local_free_list);
    if (object != nullptr) {
      return object;
    }

    _lock.lock();
    _list_swaps++;
    std::swap(_merged_local_free_list, _merged_external_free_list);
    _lock.unlock();

    object = linkedlist_pool::allocate_raw(_merged_local_free_list);
    if (object != nullptr) {
      return object;
    }
    return nullptr;
  }

  std::string to_string() {
    std::string str = "";
    float ratio = 0.0;
    if (_allocs > 0)
      ratio = _external_deallocs / float(_allocs);
    str = strprintf("Total allocs %zu, local deallocs %zu, extranl deallocs %zu (%2.1f percent), swaps %zu total pools %zu",
          _allocs, _local_deallocs, _external_deallocs, ratio * 100, _list_swaps, get_pool_count());
    return str;
  }

  linkedlist_pool* get_active_pool() const {
    return _active_pool;
  };

  size_t get_pool_count() {
    linkedlist_pool* curr_pool = &_first_pool;
    size_t pool_count = 0;
    while(curr_pool != nullptr) {
      pool_count++;
      curr_pool = curr_pool->get_next_pool();
    }
    return pool_count;
  };

  size_t get_local_free_list_size() const {
    return linkedlist_pool::get_linked_list_size(_merged_local_free_list);
  };

  size_t get_external_free_list_size() {
    _lock.lock();
    size_t size = linkedlist_pool::get_linked_list_size(_merged_external_free_list);
    _lock.unlock();
    return size;
  };

private:

  bool deallocate_object_impl(char* object, size_t threadid) {
    if (threadid == _tid) {
      _local_deallocs++;
      return linkedlist_pool::deallocate_raw(object, _merged_local_free_list);
    } else {
      _lock.lock();
      _external_deallocs++;
      bool b = linkedlist_pool::deallocate_raw(object, _merged_external_free_list);
      _lock.unlock();
      return b;
    }
    return true;
  }

  linkedlist_pool::element_header* _merged_local_free_list;

  size_t _allocs;

  size_t _local_deallocs;

  size_t _tid;

  char _padding[s_alignment_padding_bytes];

  linkedlist_pool _first_pool;

  linkedlist_pool* _active_pool;

  linkedlist_pool* _last_pool;

  spin_lock _lock;

  size_t _external_deallocs;

  size_t _list_swaps;

  linkedlist_pool::element_header* _merged_external_free_list;

  SYMPHONY_DELETE_METHOD(thread_local_fixed_pool_set(thread_local_fixed_pool_set const&));
  SYMPHONY_DELETE_METHOD(thread_local_fixed_pool_set&
      operator=(thread_local_fixed_pool_set const&));
  SYMPHONY_DELETE_METHOD(thread_local_fixed_pool_set(thread_local_fixed_pool_set const&&));
  SYMPHONY_DELETE_METHOD(thread_local_fixed_pool_set&
      operator=(thread_local_fixed_pool_set const&&));
};

class thread_local_allocator {

  enum class size_class {
    small_size = 0,

    large_size = 1,
    giant_size = 2
  };

  static default_allocator* s_default_allocator;

public:

  thread_local_allocator(size_t threadid, size_t pool_size, size_t object_size):
    _tid(threadid),
    _pool_size(pool_size),
    _default_object_size(object_size),
    _default_size_class_pools(_tid, _pool_size, _default_object_size) {
  }

  ~thread_local_allocator() {
  }

  static size_class get_small_size_class() { return size_class::small_size; }

  static size_class get_giant_size_class() { return size_class::giant_size; }

  thread_local_fixed_pool_set* get_default_size_class_pool_set() const {
    return const_cast<thread_local_fixed_pool_set*>(&_default_size_class_pools);
  }

  static default_allocator* get_oversize_allocator() {
    return s_default_allocator;
  }

  size_class find_size_class (size_t object_size) const {
    if (object_size <= _default_object_size)
      return size_class::small_size;
    return size_class::giant_size;
  }

  char* allocate(size_t object_size) {
    char* object;
    size_class object_size_class = find_size_class(object_size);

    if (object_size_class == size_class::small_size) {
      thread_local_fixed_pool_set* pool_chain = &_default_size_class_pools;
      object = pool_chain->allocate_object();
      if (object != nullptr) {
        return object;
      }
      linkedlist_pool* new_pool = new
          linkedlist_pool(_pool_size, _default_object_size, pool_chain);
      _default_size_class_pools.add_pool(new_pool);
      _default_size_class_pools.set_active_pool(new_pool);
      object = pool_chain->allocate_object();
      return object;
    }

    if (object_size_class == size_class::giant_size) {
      object = s_default_allocator->allocate_object(object_size);
      return object;
    }
    return nullptr;
  }

  static void init_default_allocator() {
    s_default_allocator = new default_allocator();
  }

  static void shutdown_default_allocator() {
    delete s_default_allocator;
  }

  static char* allocate_default(size_t object_size) {
    return s_default_allocator->allocate_object(object_size);
  }

  std::string to_string() {
    std::string str = strprintf("Thread Local Allocator %zu\n%s",
        _tid, _default_size_class_pools.to_string().c_str());
    return str;
  }

  size_t get_threadid() {
    return _tid;
  }

  SYMPHONY_DELETE_METHOD(thread_local_allocator(thread_local_allocator const&));
  SYMPHONY_DELETE_METHOD(thread_local_allocator&
      operator=(thread_local_allocator const&));
  SYMPHONY_DELETE_METHOD(thread_local_allocator(thread_local_allocator const&&));
  SYMPHONY_DELETE_METHOD(thread_local_allocator&
      operator=(thread_local_allocator const&&));

private:

  size_t _tid;

  size_t _pool_size;

  size_t _default_object_size;

  thread_local_fixed_pool_set _default_size_class_pools;

};

};
};
};
