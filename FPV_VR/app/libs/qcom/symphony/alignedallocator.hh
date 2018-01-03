// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// This file contains code snippets copied from mastermind-strategy, which is under MIT license.
#pragma once

#include <memory>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/memalloc/alignedmalloc.hh>

namespace symphony {

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template <class T, size_t Alignment>

struct aligned_allocator : public std::allocator<T>
{

SYMPHONY_GCC_IGNORE_END("-Weffc++");

  typedef typename std::allocator<T>::size_type size_type;
  typedef typename std::allocator<T>::pointer pointer;
  typedef typename std::allocator<T>::const_pointer const_pointer;

  template <class U>
  struct rebind {
    typedef aligned_allocator<U,Alignment> other;
  };

  aligned_allocator() {}

  aligned_allocator(const aligned_allocator& other) :
    std::allocator<T>(other) {}

  template <class U>
  aligned_allocator(const aligned_allocator<U,Alignment>&) {}

  ~aligned_allocator() {}

  pointer allocate(size_type n) {
    return allocate(n, const_pointer(0));
  }

  pointer allocate(size_type n, const_pointer) {
    void *p;
    p = internal::symphony_aligned_malloc(Alignment, n*sizeof(T));
    if (!p)
      throw std::bad_alloc();

    return static_cast<pointer>(p);
  }

  void deallocate(pointer p, size_type) {
    internal::symphony_aligned_free(p);
  }

};

template <class T1, size_t A1, class T2, size_t A2>
bool operator == (const aligned_allocator<T1,A1> &,
                  const aligned_allocator<T2,A2> &)
{
  return true;
}

template <class T1, size_t A1, class T2, size_t A2>
bool operator != (const aligned_allocator<T1,A1> &,
                  const aligned_allocator<T2,A2> &)
{
  return false;
}

};
