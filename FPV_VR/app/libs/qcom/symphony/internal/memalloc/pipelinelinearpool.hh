// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

namespace symphony {

namespace internal {

namespace allocatorinternal {

template<typename T> class pipeline_slab_allocator;

template<typename T>
class pipeline_fixed_size_linear_pool {

public:

  pipeline_fixed_size_linear_pool():
    _allocator(nullptr),
    _buffer(nullptr),
    _id(0),
    _pool_entries(0) {
  }

  ~pipeline_fixed_size_linear_pool() {
    delete[] _buffer;
  }

  void deallocate();

  T get_object (size_t entry_index) const {
    SYMPHONY_INTERNAL_ASSERT(
      entry_index < _pool_entries, "out of range access to the pool.");

    return _buffer[entry_index];
  }

  T& operator[](size_t entry_index) const {

    SYMPHONY_INTERNAL_ASSERT(
      entry_index < _pool_entries, "out of range access to the pool. eid %zu pool %zu", entry_index, _pool_entries);

    return _buffer[entry_index];
  }

  size_t get_id() const {
    return _id;
  }

  void initialize(size_t id,
                  size_t pool_entries,
                  pipeline_slab_allocator<T>* allocator) {
    _id = id;
    _pool_entries = pool_entries;
    _buffer = new T[_pool_entries];
    _allocator = allocator;
  }

  void set_object (T input, size_t entry_index) {
    SYMPHONY_INTERNAL_ASSERT(
      entry_index < _pool_entries, "out of range access to the pool.");

    _buffer[entry_index] = input;
  }

  SYMPHONY_DELETE_METHOD(pipeline_fixed_size_linear_pool(
      pipeline_fixed_size_linear_pool const&));
  SYMPHONY_DELETE_METHOD(pipeline_fixed_size_linear_pool&
      operator=(pipeline_fixed_size_linear_pool const&));
  SYMPHONY_DELETE_METHOD(pipeline_fixed_size_linear_pool(
      pipeline_fixed_size_linear_pool const&&));
  SYMPHONY_DELETE_METHOD(pipeline_fixed_size_linear_pool&
      operator=(pipeline_fixed_size_linear_pool const&&));

  friend pipeline_slab_allocator<T>;

private:

  pipeline_slab_allocator<T>*  _allocator;
  T*                           _buffer;
  size_t                       _id;
  size_t                       _pool_entries;
};

};
};
};
