// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <string>

#include <symphony/internal/memalloc/pool.hh>
#include <symphony/internal/util/debug.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class slab_allocator;

class concurrent_bump_pool: public pool {
public:

  concurrent_bump_pool(size_t pool_byte_size, char* pool_buffer):
    _buffer(pool_buffer),
    _pointer(nullptr),
    _allocator(nullptr),
    _total_allocs(0),
    _current_allocs(0),
    _pool_byte_size(pool_byte_size),
    _id(0),
    _ref_count(0),
    _concurrent(false) {
    SYMPHONY_INTERNAL_ASSERT(pool_byte_size > 0,
                         "Cannot allocate an empty bump pool");
  }

  concurrent_bump_pool():
    _buffer(nullptr),
    _pointer(nullptr),
    _allocator(nullptr),
    _total_allocs(0),
    _current_allocs(0),
    _pool_byte_size(0),
    _id(0),
    _ref_count(0),
    _concurrent(false) {
  }

  ~concurrent_bump_pool() {
  }

  void initialize(size_t id,
                  size_t pool_byte_size,
                  char* pool_buffer,
                  slab_allocator* allocator) {
    _id = id;
    _pool_byte_size = pool_byte_size;
    _buffer = pool_buffer;
    _allocator = allocator;
    _pointer = _buffer;
  }

  virtual bool deallocate_object(char* object) {
    return deallocate_object_impl(object);
  }

  virtual bool deallocate_object(char* object, size_t threadid) {
    SYMPHONY_UNUSED(threadid);
    return deallocate_object_impl(object);
  }

  void deallocate();

  char* allocate_object (size_t size) {
    char* object = nullptr;
    if (_pointer + size + pool::s_embedded_pool_offset <=
        _buffer + _pool_byte_size) {
      _pointer = embed_pool(_pointer, this);
      object = _pointer;
      _pointer += size;
      _current_allocs++;
      return object;
    }
    return nullptr;
  }

  bool allocate (size_t ref_count = 0) {
    _pointer = _buffer;
    _total_allocs += _current_allocs;
    if (ref_count > 0) {
      _ref_count.store(ref_count, symphony::mem_order_release);
      _concurrent = true;
    }
    else {
      _concurrent = false;
    }
    _current_allocs = 0;
    return true;
  }

  size_t get_id() const {
    return _id;
  }

  bool is_owned(char* object) const {
    return (object >= _buffer && object < _buffer + _pool_byte_size);
  }

  slab_allocator* get_allocator() const {return _allocator;}

  size_t get_current_allocs() const { return _current_allocs; };

  virtual std::string to_string() {
    std::string str = strprintf("Concurrent Bump pool info: pool[%zu] %p allocs %zu RC %zu pointer = %p size = %zu",
                                _id, _buffer, _total_allocs,
                                _ref_count.load(symphony::mem_order_relaxed), _pointer, _pool_byte_size);
    return str;
  }

  SYMPHONY_DELETE_METHOD(concurrent_bump_pool(concurrent_bump_pool const&));
  SYMPHONY_DELETE_METHOD(concurrent_bump_pool(concurrent_bump_pool const&&));
  SYMPHONY_DELETE_METHOD(concurrent_bump_pool& operator=(concurrent_bump_pool const&));
  SYMPHONY_DELETE_METHOD(concurrent_bump_pool&& operator=(concurrent_bump_pool const&&));

private:

  bool deallocate_object_impl(char* object);

  char* _buffer;

  char* _pointer;

  slab_allocator* _allocator;

  size_t _total_allocs;

  size_t _current_allocs;

  size_t _pool_byte_size;

  size_t _id;

  std::atomic<size_t> _ref_count;

  bool _concurrent;

  friend slab_allocator;
};

};

};

};
