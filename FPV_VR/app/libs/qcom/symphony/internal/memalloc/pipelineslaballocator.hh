// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/memalloc/alignedmalloc.hh>
#include <symphony/internal/util/ffs.hh>
#include <symphony/internal/util/memorder.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

template<typename T>
class pipeline_slab_allocator {

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

        size_t bit_index = symphony_ffs(~current_bitmap);

        if (bit_index == 0 || bit_index > pool_count)
          return false;

        --bit_index;

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

  explicit pipeline_slab_allocator(
    size_t pool_entries):
    _id_assigner(),
    _pool_count(pool_id_assigner::get_max_ids()),
    _pool_entries(pool_entries),
    _pools(nullptr) {

    _pools = new pipeline_fixed_size_linear_pool<T>[_pool_count];

    for (size_t i = 0;i < _pool_count; i++) {
      _pools[i].initialize(i, pool_entries, this);
    }
  }

  pipeline_fixed_size_linear_pool<T>* allocate_pool() {
    size_t id;
    if (_id_assigner.allocate(id, _pool_count)) {
      return &_pools[id];
    }
    return nullptr;
  }

  static size_t get_max_pool_id() {
    return pool_id_assigner::get_max_ids();
  }

  ~pipeline_slab_allocator() {

    delete []_pools;
  }

private:

  void deallocate_pool(pipeline_fixed_size_linear_pool<T>* pool) {
    size_t id = pool->get_id();
    if(id < get_max_pool_id()) {
      _id_assigner.deallocate(pool->get_id());
    }
  }

  pool_id_assigner _id_assigner;

  size_t _pool_count;

  size_t _pool_entries;

  pipeline_fixed_size_linear_pool<T>* _pools;

  friend pipeline_fixed_size_linear_pool<T>;

  SYMPHONY_DELETE_METHOD(pipeline_slab_allocator(pipeline_slab_allocator const&));
  SYMPHONY_DELETE_METHOD(pipeline_slab_allocator& operator=(pipeline_slab_allocator const&));
  SYMPHONY_DELETE_METHOD(pipeline_slab_allocator(pipeline_slab_allocator const&&));
  SYMPHONY_DELETE_METHOD(pipeline_slab_allocator& operator=(pipeline_slab_allocator const&&));
};

template<typename T>
inline void
pipeline_fixed_size_linear_pool<T>::deallocate() {
  if (_allocator != nullptr)
    _allocator->deallocate_pool(this);
}

};
};
};
