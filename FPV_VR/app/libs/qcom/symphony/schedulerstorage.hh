// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/scopedstorage.hh>

#include <symphony/internal/storage/schedulerstorage.hh>

namespace symphony {

template<typename T, class Allocator = std::allocator<T> >
class scheduler_storage_ptr :
    public scoped_storage_ptr<scheduler_storage_ptr, T, Allocator>
{
public:
  typedef Allocator allocator_type;
  scheduler_storage_ptr() {}
#if __cplusplus >= 201103L
private:
  friend class scoped_storage_ptr< ::symphony::scheduler_storage_ptr, T, Allocator>;
#else
public:
#endif
  inline static int key_create(internal::storage_key* key,
                               void (*dtor)(void*)) {
    return sls_key_create(key, dtor);
  }
  inline static int set_specific(internal::storage_key key,
                                 void const* value) {
    return sls_set_specific(key, value);
  }
  inline static void* get_specific(internal::storage_key key) {
    return sls_get_specific(key);
  }
};

};
