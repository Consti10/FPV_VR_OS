// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <string>

#include <symphony/internal/memalloc/alignedmalloc.hh>
#include <symphony/internal/util/strprintf.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class thread_local_fixed_pool_set;

typedef thread_local_fixed_pool_set owner_pool_t;

class linkedlist_pool {

  struct element_header {

    element_header* _next_empty;

    owner_pool_t* _pool;
  };

  static constexpr size_t s_header_offset =
      sizeof(element_header);

public:

  linkedlist_pool(size_t pool_size, size_t object_size, owner_pool_t* pool):
    _pool_size(pool_size),

    _element_size( (static_cast<int>(sizeof (element_header) + object_size + 3) / 4 ) * 4),
    _owner_pool(pool),
    _total_deallocs(0),
    _buffer(nullptr),
    _next_pool(nullptr),
    _prev_pool(nullptr) {

    _buffer = reinterpret_cast<char*>(symphony_aligned_malloc(128, _pool_size * _element_size));
    if (_buffer == nullptr) {
      throw std::bad_alloc();
    }
    initialize_buffer();
  }

  element_header* get_first_entry() {
    return reinterpret_cast<element_header*>(_buffer);
  }

  virtual ~linkedlist_pool() {
    symphony_aligned_free(_buffer);
  }

  SYMPHONY_DELETE_METHOD(linkedlist_pool(linkedlist_pool const&));
  SYMPHONY_DELETE_METHOD(linkedlist_pool& operator=(linkedlist_pool const&));
  SYMPHONY_DELETE_METHOD(linkedlist_pool(linkedlist_pool const&&));
  SYMPHONY_DELETE_METHOD(linkedlist_pool& operator=(linkedlist_pool const&&));

  void initialize_buffer() {
    char* buffer = _buffer;
    for (size_t i = 0; i < _pool_size; i++) {
      auto element = reinterpret_cast<element_header*>(buffer);
      auto next_element = reinterpret_cast<element_header*>(buffer + _element_size);

      if (i == _pool_size - 1) {
        element->_next_empty = nullptr;
      } else {
        element->_next_empty = next_element;
      }

      element->_pool = _owner_pool;
      buffer = reinterpret_cast<char*>(next_element);
    }
  }

  inline static char* allocate_raw(element_header*& shared_next_empty) {
    if (shared_next_empty != nullptr) {

      char* object = (reinterpret_cast<char*>(shared_next_empty) + s_header_offset);

      shared_next_empty = shared_next_empty->_next_empty;

      return object;
    }
    return nullptr;
  }

  inline static owner_pool_t* extract_owner_pool(char* object) {
    element_header* element = reinterpret_cast<element_header*>(object - s_header_offset);
    return element->_pool;
  }

  inline static bool deallocate_raw(char* memory, element_header*& shared_next_empty) {

    element_header* element = reinterpret_cast<element_header*>(memory - s_header_offset);
    element->_next_empty = shared_next_empty;

    shared_next_empty = element;
    return false;
  }

  static size_t get_linked_list_size(element_header* next_empty) {
    size_t count = 0;
    while (next_empty != nullptr) {
      next_empty = next_empty->_next_empty;
      count++;
    }
    return count;
  }

  size_t get_total_deallocs() {
    return _total_deallocs;
  }

  inline linkedlist_pool* get_next_pool() {
    return _next_pool;
  }

  inline linkedlist_pool* get_prev_pool() {
    return _prev_pool;
  }

  inline void set_next_pool(linkedlist_pool* next) {
    _next_pool = next;
  }

  inline void set_prev_pool(linkedlist_pool* prev) {
    _prev_pool = prev;
  }

private:

  size_t _pool_size;

  size_t _element_size;

  owner_pool_t* _owner_pool;

  size_t _total_deallocs;

  char* _buffer;

  linkedlist_pool* _next_pool;

  linkedlist_pool* _prev_pool;

  friend thread_local_fixed_pool_set;
};

};

};

};
