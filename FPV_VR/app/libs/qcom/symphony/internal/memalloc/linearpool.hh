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

template<size_t EntrySize, size_t Entries>
class fixed_size_linear_pool {
public:

  fixed_size_linear_pool(): _buffer(nullptr){
    static_assert(EntrySize > 0 && Entries > 0,
                  "Both Entries and EntrySize must be larger than zero.");
    _buffer = reinterpret_cast<char*>(symphony_aligned_malloc(128, EntrySize * Entries));
    if (_buffer == nullptr) {
      throw std::bad_alloc();
    }

  }

  ~fixed_size_linear_pool() {
    symphony_aligned_free(_buffer);
  }

  char* allocate_object (size_t entry_index) {
    if (entry_index < Entries) {
      return _buffer + entry_index * EntrySize;
    }
    return nullptr;
  }

  bool deallocate_object (char* object) {
    return true;
    SYMPHONY_UNUSED(object);
  }

  std::string to_string() {
    std::string str = strprintf("Linear pool: buffer %p entry size %zu Entries %zu",
        _buffer, EntrySize, Entries);
    return str;
  }

  char* get_buffer() {return _buffer;};

  SYMPHONY_DELETE_METHOD(fixed_size_linear_pool(
      fixed_size_linear_pool const&));
  SYMPHONY_DELETE_METHOD(fixed_size_linear_pool&
      operator=(fixed_size_linear_pool const&));
  SYMPHONY_DELETE_METHOD(fixed_size_linear_pool(
      fixed_size_linear_pool const&&));
  SYMPHONY_DELETE_METHOD(fixed_size_linear_pool&
      operator=(fixed_size_linear_pool const&&));

private:

  char* _buffer;
};

};

};

};
