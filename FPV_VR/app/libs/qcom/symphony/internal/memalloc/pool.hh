// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/util/macros.hh>

namespace symphony {

namespace internal {

namespace allocatorinternal {

class pool {

public:
  pool () {};
  virtual ~pool() {};

  virtual bool deallocate_object(char* object) = 0;

  virtual bool deallocate_object(char* object, size_t threadid) = 0;

  static pool* get_embedded_pool(char* object) {
    using pool_elem = embedded_pool_element;
    pool_elem* element = reinterpret_cast<pool_elem*>(object - get_embedded_pool_header_size());
    return element->_pool;
  }

  static char* embed_pool(char* object, pool* p) {
    using pool_elem = embedded_pool_element;
    pool_elem* element = reinterpret_cast<pool_elem*> (object);
    element->_pool = p;
    return object + get_embedded_pool_header_size();
  }

  static constexpr size_t get_embedded_pool_header_size() {
    return s_embedded_pool_offset;
  }

  virtual std::string to_string() = 0;

protected:

  static constexpr size_t s_embedded_pool_offset = sizeof(pool*) * 2;

  struct embedded_pool_element {

    char _padding[s_embedded_pool_offset - sizeof(pool*)];
    pool* _pool;
  };

  SYMPHONY_DELETE_METHOD(pool(pool const&));
  SYMPHONY_DELETE_METHOD(pool& operator=(pool const&));
  SYMPHONY_DELETE_METHOD(pool(pool const&&));
  SYMPHONY_DELETE_METHOD(pool& operator=(pool const&&));
};

};

};

};
