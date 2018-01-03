// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <string>

#include <symphony/internal/memalloc/pool.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class default_allocator: public pool {

public:

  default_allocator() {
  }

  char* allocate_object(size_t size) {

    char* wrapper = new char[size + pool::s_embedded_pool_offset];

    char* object = embed_pool(wrapper, this);

    return object;
  }

  virtual bool deallocate_object(char* object) {
    return deallocate_object_impl(object);
  }

  virtual bool deallocate_object(char* object, size_t threadid) {
    SYMPHONY_UNUSED(threadid);
    return deallocate_object_impl(object);
  }

  virtual std::string to_string() {
    return "DEFAULT";
  }

private:

  bool deallocate_object_impl(char* object) {

    char* wrapper = object - pool::s_embedded_pool_offset;

    delete [] wrapper;
    return true;
  }

  SYMPHONY_DELETE_METHOD(default_allocator(default_allocator const&));
  SYMPHONY_DELETE_METHOD(default_allocator& operator=(default_allocator const&));
  SYMPHONY_DELETE_METHOD(default_allocator(default_allocator const&&));
  SYMPHONY_DELETE_METHOD(default_allocator& operator=(default_allocator const&&));
};

};
};
};
