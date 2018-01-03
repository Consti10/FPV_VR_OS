// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

namespace symphony {
namespace internal {

template<typename T> class pipeline_pool_buckets;

template<typename T>
class pipeline_chunk_list_node{

  T                            _data;
  pipeline_chunk_list_node<T>* _next;

public:
explicit pipeline_chunk_list_node(T data):
   _data(data),
   _next(nullptr) { }

  SYMPHONY_DELETE_METHOD(pipeline_chunk_list_node(pipeline_chunk_list_node const& other));
  SYMPHONY_DELETE_METHOD(pipeline_chunk_list_node(pipeline_chunk_list_node&& other));
  SYMPHONY_DELETE_METHOD(pipeline_chunk_list_node& operator=(pipeline_chunk_list_node const& other));
  SYMPHONY_DELETE_METHOD(pipeline_chunk_list_node& operator=(pipeline_chunk_list_node&& other));

  template<typename C> friend class pipeline_pool_buckets;

};

template<typename T>
class pipeline_pool_buckets{

  static SYMPHONY_CONSTEXPR_CONST size_t s_chunk_size = 1024;
  static SYMPHONY_CONSTEXPR_CONST size_t s_chunk_size_mask = s_chunk_size - 1;
  static SYMPHONY_CONSTEXPR_CONST size_t s_chunk_size_bits = 10;

  pipeline_chunk_list_node<std::array<T, s_chunk_size>*>* _first_chunk;
  pipeline_chunk_list_node<std::array<T, s_chunk_size>*>* _last_chunk;
  size_t                                                  _num_elems;

public:

  pipeline_pool_buckets():
    _first_chunk(nullptr),
    _last_chunk(nullptr),
    _num_elems(0) { }

  ~pipeline_pool_buckets() {
    auto p = _first_chunk;
    while(p) {
      auto n = p->_next;
      delete p->_data;
      delete p;
      p = n;
    }
  }

  void push_back(T elem) {
    auto elem_id = _num_elems & s_chunk_size_mask;

    if(elem_id == 0) {
      auto curr_chunk =
        new pipeline_chunk_list_node<std::array<T, s_chunk_size>*>(
          new std::array<T, s_chunk_size>());

      if(_first_chunk == nullptr) {
        _first_chunk = curr_chunk;
      }
      if(_last_chunk !=nullptr) {
        _last_chunk->_next = curr_chunk;
      }
      _last_chunk = curr_chunk;
    }
    (*_last_chunk->_data)[elem_id] = elem;
    _num_elems ++;
  }

  T& operator[](size_t id) {
    SYMPHONY_INTERNAL_ASSERT(id <= _num_elems, "out of range bucket access");

    auto elem_id = id & s_chunk_size_mask;
    auto chunk_id = id >> s_chunk_size_bits;

    auto curr_chunk = _first_chunk;
    for(size_t i = 0; i < chunk_id; i ++) {
      SYMPHONY_INTERNAL_ASSERT(curr_chunk != nullptr, "pool buckets chunk is broken");
      curr_chunk = curr_chunk->_next;
    }

    return (*curr_chunk->_data)[elem_id];
  }

  T& back() {
    SYMPHONY_INTERNAL_ASSERT(_last_chunk != nullptr, "pool is empty");
    auto elem_id = (_num_elems - 1) & s_chunk_size_mask;
    return (*_last_chunk->_data)[elem_id];
  }

  size_t size() {
    return _num_elems;
  }

  SYMPHONY_DELETE_METHOD(pipeline_pool_buckets(pipeline_pool_buckets const& other));
  SYMPHONY_DELETE_METHOD(pipeline_pool_buckets(pipeline_pool_buckets&& other));
  SYMPHONY_DELETE_METHOD(pipeline_pool_buckets& operator=(pipeline_pool_buckets const& other));
  SYMPHONY_DELETE_METHOD(pipeline_pool_buckets& operator=(pipeline_pool_buckets&& other));
};

};
};
