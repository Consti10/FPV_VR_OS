// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <stdexcept>

#include <symphony/internal/memalloc/pipelinelinearpool.hh>
#include <symphony/internal/memalloc/pipelineslaballocator.hh>
#include <symphony/internal/patterns/pipeline/pipelinepoolbucket.hh>

namespace symphony {
namespace internal {

typedef enum pipeline_launch_type{
  with_sliding_window = 0,
  without_sliding_window
} pipeline_launch_type;

class stagebuffer{

protected:
  size_t _size;

public:

  explicit stagebuffer(size_t size):_size(size) {}

  virtual ~stagebuffer() {}

  virtual void allocate_pool() = 0;

  size_t get_size() const {return _size; }

  virtual void reset_pool() = 0;

  SYMPHONY_DELETE_METHOD(stagebuffer(stagebuffer const& other));
  SYMPHONY_DELETE_METHOD(stagebuffer(stagebuffer&& other));
  SYMPHONY_DELETE_METHOD(stagebuffer& operator=(stagebuffer const& other));
  SYMPHONY_DELETE_METHOD(stagebuffer& operator=(stagebuffer&& other));

};

template<typename T>
class ringbuffer: public stagebuffer {
  T*  _buffer;

public:

  explicit ringbuffer(size_t s):
    stagebuffer(s),
    _buffer(new T[s]) {
  }

  virtual ~ringbuffer() {
    delete[] _buffer;
    _buffer = nullptr;
  }

  virtual void allocate_pool() {
    SYMPHONY_UNREACHABLE("Cannot allocate pool for ringbuffer.");
  }

  T& operator[](size_t id) {
    SYMPHONY_INTERNAL_ASSERT(id < _size, "Out of range buffer access.");
    return _buffer[id];
  };

  size_t get_buffer_index(size_t iter_id) const {
    return iter_id % _size;
  }

  size_t get_size() const {
    return _size;
  }

  void set(size_t id, T value) {
    SYMPHONY_INTERNAL_ASSERT(id < _size, "Out of range buffer access.");
    _buffer[id] = value;
  }

  virtual void reset_pool() {
    SYMPHONY_UNREACHABLE("Cannot allocate pool for ringbuffer.");
  }

  SYMPHONY_DELETE_METHOD(ringbuffer(ringbuffer const& other));
  SYMPHONY_DELETE_METHOD(ringbuffer(ringbuffer&& other));
  SYMPHONY_DELETE_METHOD(ringbuffer& operator=(ringbuffer const& other));
  SYMPHONY_DELETE_METHOD(ringbuffer& operator=(ringbuffer&& other));
};

template<>
class ringbuffer<void>: public stagebuffer {

public:

  explicit ringbuffer(size_t s):
    stagebuffer(s) {
    SYMPHONY_INTERNAL_ASSERT(false,
      "Cannot construct a stage buffer with data of type void");
  }

  virtual ~ringbuffer() {
  }

  SYMPHONY_DELETE_METHOD(ringbuffer(ringbuffer const& other));
  SYMPHONY_DELETE_METHOD(ringbuffer(ringbuffer&& other));
  SYMPHONY_DELETE_METHOD(ringbuffer& operator=(ringbuffer const& other));
  SYMPHONY_DELETE_METHOD(ringbuffer& operator=(ringbuffer&& other));
};

template<typename T>
class dynamicbuffer:public stagebuffer{

 static symphony::mem_order const pool_mem_order = symphony::mem_order_relaxed;

public:
  using size_type         = size_t;
  using pipe_slab_alloc   = allocatorinternal::pipeline_slab_allocator<T>;
  using pipe_fsl_pool = allocatorinternal::pipeline_fixed_size_linear_pool<T>;

private:
  static SYMPHONY_CONSTEXPR_CONST size_type s_bucket_size = 64;
#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
  size_t                                                  _num_alloc;
  size_t                                                  _num_dalloc;
  size_t                                                  _num_reset;
#endif

  std::atomic<size_t>                                     _first_pool;
  std::atomic<size_t>                                     _num_pools;
  pipeline_pool_buckets<std::array<pipe_fsl_pool*, s_bucket_size>*>
                                                          _pool_buckets;
  size_t                                                  _pool_entries;
  pipe_slab_alloc                                         _slab;

public:

  explicit dynamicbuffer(size_t pool_entries):
#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
   _num_alloc(0),
   _num_dalloc(0),
   _num_reset(0),
#endif
   stagebuffer(pool_entries),
   _first_pool(0),
   _num_pools(0),
   _pool_buckets(),
   _pool_entries(pool_entries),
  _slab(pool_entries) { }

  virtual ~dynamicbuffer() {

#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
   SYMPHONY_ALOG("DYNAMIC BUFFER %p, %zu out of %zu alloc from the slab, %zu resets",
     this, _num_alloc-_num_dalloc, _num_alloc, _num_reset);
#endif

    for(size_t i = 0; i < _pool_buckets.size(); i ++) {
      auto bucket = _pool_buckets[i];
      if(bucket == nullptr)
        continue;

      for(auto const& pool : *bucket) {

       if(pool == nullptr)
         continue;
       if(pool->get_id() == pipe_slab_alloc::get_max_pool_id() + 1)
          delete pool;
      }
      delete bucket;
    }
  }

  virtual void allocate_pool() {

    pipe_fsl_pool* p = _slab.allocate_pool();

#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
    _num_alloc ++;
#endif

    if(p == nullptr) {
#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
    _num_dalloc ++;
#endif
      p = new pipe_fsl_pool;
      p->initialize(
        pipe_slab_alloc::get_max_pool_id() + 1, _pool_entries, nullptr);
    }

    size_t elem_id = _num_pools.load(pool_mem_order) % s_bucket_size;

    if(elem_id == 0) {
      auto new_bucket = new std::array<pipe_fsl_pool*, s_bucket_size>();
      new_bucket->fill(nullptr);
      _pool_buckets.push_back(new_bucket);
    }

    (*_pool_buckets.back())[elem_id] = p;
    _num_pools.fetch_add(1, pool_mem_order);

#if defined(SYMPHONY_PIPELINE_DEBUG_BUFFER) && defined(DEBUG_PIPELINE)
        SYMPHONY_ALOG(
          "\033[31m allocating pool %p(%zu) in %p, first_pool %zu, num_pools %zu, pool_entries %zu, elem_id %zu\033[0m\n",
          p, p->get_id(), this, _first_pool.load(pool_mem_order),
          _num_pools.load(pool_mem_order), _pool_entries, elem_id) ;
#endif
  }

  size_t get_buffer_index(size_t iter_id) const {

    return iter_id;
  }

  size_t get_size() const {
    return _pool_entries;
  }

  virtual void reset_pool() {
#ifdef SYMPHONY_PIPELINE_DEBUG_BUFFER
    _num_reset ++;
#endif
    size_t elem_id   =
      _first_pool.load(pool_mem_order) % s_bucket_size;
    size_t bucket_id =
      _first_pool.load(pool_mem_order) / s_bucket_size;

    SYMPHONY_INTERNAL_ASSERT(_pool_buckets[bucket_id] != nullptr,
      "pool buckets should not be empty.");

    (*_pool_buckets[bucket_id])[elem_id]->deallocate();
    (*_pool_buckets[bucket_id])[elem_id] = nullptr;

    if(elem_id == s_bucket_size - 1) {
      delete _pool_buckets[bucket_id];
      _pool_buckets[bucket_id] = nullptr;
    }
    _first_pool.fetch_add(1, pool_mem_order) ;
  }

  T& operator[](size_t id) {
    SYMPHONY_API_THROW_CUSTOM(
      id < _num_pools.load(pool_mem_order) * _pool_entries &&
      id >= _first_pool.load(pool_mem_order) * _pool_entries,
      std::out_of_range,
      "Out of range buffer access.");

    size_t pool_id = id / _pool_entries;
    size_t bucket_id = pool_id / s_bucket_size;
    pool_id = pool_id % s_bucket_size;
    size_t elem_id = id % _pool_entries;

    return (*(*_pool_buckets[bucket_id])[pool_id])[elem_id];
  };

  SYMPHONY_DELETE_METHOD(dynamicbuffer(dynamicbuffer const& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer(dynamicbuffer&& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer& operator=(dynamicbuffer const& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer& operator=(dynamicbuffer&& other));
};

template<>
class dynamicbuffer<void> : public stagebuffer {

public:

  explicit dynamicbuffer(size_t pool_entries) : stagebuffer(pool_entries) {
    SYMPHONY_INTERNAL_ASSERT(false,
      "Cannot construct a dynamic buffer with data of type void.");
  }

  virtual ~dynamicbuffer() {
  }

  SYMPHONY_DELETE_METHOD(dynamicbuffer(dynamicbuffer const& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer(dynamicbuffer&& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer& operator=(dynamicbuffer const& other));
  SYMPHONY_DELETE_METHOD(dynamicbuffer& operator=(dynamicbuffer&& other));
};

};
};
