// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <string>

#include <symphony/internal/memalloc/alignedmalloc.hh>
#include <symphony/internal/memalloc/concurrentbumppool.hh>
#include <symphony/internal/util/ffs.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class slab_allocator {

private:

  class pool_id_assigner {

    typedef uint32_t element_type;

  public:

    pool_id_assigner():
      _bitmap(0) {
    }

    ~pool_id_assigner() {
    }

    inline bool allocate(size_t& id, size_t pool_count) {
      while (true) {

        element_type current_bitmap = _bitmap.load(symphony::mem_order_relaxed);

        size_t bit_number = symphony_ffs(~current_bitmap);

        if (bit_number == 0 || bit_number > pool_count)
          return false;

        size_t bit_index = bit_number - 1;

        element_type mask = 1 << bit_index;
        current_bitmap = _bitmap.fetch_or(mask, symphony::mem_order_acq_rel);

        if ((current_bitmap & mask) == 0)  {
          id = bit_index;
          return true;
        }

      }
    }

    inline void deallocate(size_t id) {
      size_t bit_index = id;
      _bitmap.fetch_and(~(1 << bit_index), symphony::mem_order_acq_rel);
    }

    element_type get_ids() const {
      return _bitmap.load(symphony::mem_order_relaxed);
    }

    static size_t get_max_ids() {
      return sizeof(element_type) * 8;
    }

  private:

    std::atomic<element_type> _bitmap;
  };

public:

  explicit slab_allocator(size_t pool_byte_size = 1024, size_t pool_count = 10):
    _pools(),
    _id_assigner(),
    _buffer(nullptr),
    _pool_byte_size(pool_byte_size),
    _pool_count(pool_count),
    _buffer_size(_pool_byte_size * _pool_count) {

      SYMPHONY_INTERNAL_ASSERT(pool_count <= pool_id_assigner::get_max_ids(),
                           "Allocating too many bump pools");
      SYMPHONY_INTERNAL_ASSERT(pool_count > 0,
                           "Need at least one pool in the slab");

      _buffer = new char[_pool_byte_size * _pool_count];
      _pools = new concurrent_bump_pool[_pool_count];

      for (size_t i = 0;i < _pool_count; i++) {
        _pools[i].initialize(i,
                             _pool_byte_size,
                             _buffer + i * _pool_byte_size,
                             this);
    }
  }

  concurrent_bump_pool* allocate_pool(size_t ref_count = 0) {
    size_t id;
    if (_id_assigner.allocate(id, _pool_count)) {
      _pools[id].allocate(ref_count);
      return &_pools[id];
    }
    SYMPHONY_INTERNAL_ASSERT(false, "No pool left!");
    return nullptr;
  }

  ~slab_allocator() {

    delete [] _buffer;
    delete [] _pools;
  }
  SYMPHONY_DELETE_METHOD(slab_allocator(slab_allocator const&));
  SYMPHONY_DELETE_METHOD(slab_allocator& operator=(slab_allocator const&));
  SYMPHONY_DELETE_METHOD(slab_allocator(slab_allocator const&&));
  SYMPHONY_DELETE_METHOD(slab_allocator& operator=(slab_allocator const&&));

  std::string to_string() {
    std::string str = "Slab allocator info: ";
    str += strprintf("buffer %p %d\n", _buffer, int(_id_assigner.get_ids()));
    for (size_t i = 0;i < _pool_count; i++) {
      str += strprintf("Pool[%zu]: %s\n", i, _pools[i].to_string().c_str());
    }
    return str;
  }

private:

  bool find_object_pool_id(char* object, size_t& pool_id) {
    if (object < _buffer ||
        object >= _buffer + _buffer_size)
      return false;
    pool_id = (object - _buffer) / _pool_byte_size;
    return true;
  }

  void deallocate_pool(concurrent_bump_pool* pool) {
    _id_assigner.deallocate(pool->get_id());
  }

  concurrent_bump_pool* _pools;

  pool_id_assigner _id_assigner;

  char* _buffer;

  size_t _pool_byte_size;

  size_t _pool_count;

  size_t _buffer_size;

  friend concurrent_bump_pool;

};

inline bool
concurrent_bump_pool::deallocate_object_impl(char* object) {

  size_t ref_count = _ref_count.fetch_sub(1, symphony::mem_order_relaxed);
  if (ref_count == 1) {
    _pointer = _buffer;
    _total_allocs += _current_allocs;
    _current_allocs = 0;
    if (_allocator != nullptr)
      _allocator->deallocate_pool(this);
    return true;
  }
  return false;
  SYMPHONY_UNUSED(object);
}

inline void
concurrent_bump_pool::deallocate() {
  if (!_concurrent)
    SYMPHONY_FATAL("An RCed pool must not be deallocated explicitly!");
  _pointer = _buffer;
  if (_allocator != nullptr)
    _allocator->deallocate_pool(this);
}

};

};

};
