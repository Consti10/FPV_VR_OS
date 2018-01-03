// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/scopedstorage.hh>

#include <symphony/internal/storage/threadstorage.hh>

namespace symphony {

template<typename T, class Allocator = std::allocator<T> >
class thread_storage_ptr :
    public scoped_storage_ptr<thread_storage_ptr, T, Allocator>
{
public:
  typedef Allocator allocator_type;
  thread_storage_ptr() {}
#if __cplusplus >= 201103L
private:
  friend class scoped_storage_ptr< ::symphony::thread_storage_ptr, T, Allocator>;
#else
public:
#endif
  inline static int key_create(internal::storage_key* key,
                               void (*dtor)(void*)) {
    return tls_key_create(key, dtor);
  }
  inline static int set_specific(internal::storage_key key,
                                 void const* value) {
    return tls_set_specific(key, value);
  }
  inline static void* get_specific(internal::storage_key key) {
    return tls_get_specific(key);
  }
};

};
