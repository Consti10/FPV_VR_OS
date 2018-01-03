// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/util/debug.hh>

#if !defined(_MSC_VER)
  #define SYMPHONY_SIZEOF_MWORD __SIZEOF_SIZE_T__
#else
  #if _M_X64
    #define SYMPHONY_SIZEOF_MWORD 8
  #else
    #define SYMPHONY_SIZEOF_MWORD 4
  #endif
#endif

#define SYMPHONY_SIZEOF_DMWORD (SYMPHONY_SIZEOF_MWORD * 2)

namespace symphony {
namespace internal {

template<size_t N, typename T>
class atomicops {
public:

  static inline T
  fetch_add(T* dest, const T& incr,
            std::memory_order m = std::memory_order_seq_cst)
  {
    SYMPHONY_INTERNAL_ASSERT(m == std::memory_order_seq_cst ||
                         m == std::memory_order_acq_rel ||
                         m == std::memory_order_relaxed,
                         "Invalid memory order %u", m);
#ifdef _MSC_VER
    return std::atomic_fetch_add_explicit(reinterpret_cast<std::atomic<T>*>(dest), incr, m);
#else
    return __atomic_fetch_add(dest, incr, m);
#endif
  }
  static inline T
  fetch_sub(T* dest, const T& incr,
            std::memory_order m = std::memory_order_seq_cst)
  {
    SYMPHONY_INTERNAL_ASSERT(m == std::memory_order_seq_cst ||
                         m == std::memory_order_acq_rel ||
                         m == std::memory_order_relaxed,
                         "Invalid memory order %u", m);
#ifdef _MSC_VER
    return std::atomic_fetch_sub_explicit(reinterpret_cast<std::atomic<T>*>(dest), incr, m);
#else
    return __atomic_fetch_sub(dest, incr, m);
#endif
  }
  static inline T
  fetch_and(T* dest, const T& incr,
            std::memory_order m = std::memory_order_seq_cst)
  {
    SYMPHONY_INTERNAL_ASSERT(m == std::memory_order_seq_cst ||
                         m == std::memory_order_acq_rel ||
                         m == std::memory_order_relaxed,
                         "Invalid memory order %u", m);
#ifdef _MSC_VER
    return std::atomic_fetch_and_explicit(reinterpret_cast<std::atomic<T>*>(dest), incr, m);
#else
    return __atomic_fetch_and(dest, incr, m);
#endif
  }
  static inline T
  fetch_or(T* dest, const T& incr,
           std::memory_order m = std::memory_order_seq_cst)
  {
    SYMPHONY_INTERNAL_ASSERT(m == std::memory_order_seq_cst ||
                         m == std::memory_order_acq_rel ||
                         m == std::memory_order_relaxed,
                         "Invalid memory order %u", m);
#ifdef _MSC_VER
    return std::atomic_fetch_or_explicit(reinterpret_cast<std::atomic<T>*>(dest), incr, m);
#else
    return __atomic_fetch_or(dest, incr, m);
#endif
  }
  static inline T
  fetch_xor(T* dest, const T& incr,
            std::memory_order m = std::memory_order_seq_cst)
  {
    SYMPHONY_INTERNAL_ASSERT(m == std::memory_order_seq_cst ||
                         m == std::memory_order_acq_rel ||
                         m == std::memory_order_relaxed,
                         "Invalid memory order %u", m);
#ifdef _MSC_VER
    return std::atomic_fetch_xor_explicit(reinterpret_cast<std::atomic<T>*>(dest), incr, m);
#else
    return __atomic_fetch_xor(dest, incr, m);
#endif
  }
};

static inline void symphony_atomic_thread_fence(std::memory_order m){
  #ifdef _MSC_VER
    return std::atomic_thread_fence(m);
  #else
    return __atomic_thread_fence(m);
  #endif
}

};

};

#if defined(i386) || defined(__i386) || defined(__i386__) ||defined(__x86_64__)
#define SYMPHONY_HAS_ATOMIC_DMWORD 1
namespace symphony {
namespace internal {
#ifdef __x86_64__
#if defined(__clang__) && __SIZEOF_INT128__ < 16
  typedef __uint128_t symphony_dmword_t;
#else
  typedef unsigned __int128 symphony_dmword_t;
#endif
#else
  typedef uint64_t symphony_dmword_t;
#endif
};
};

#elif defined(__ARM_ARCH_7A__)
namespace symphony {
namespace internal {
#ifdef __LP64__

#define SYMPHONY_HAS_ATOMIC_DMWORD 0
  typedef unsigned __int128 symphony_dmword_t;
#else
#define SYMPHONY_HAS_ATOMIC_DMWORD 1
  typedef uint64_t symphony_dmword_t;
#endif
};
};
#elif defined(__aarch64__)
namespace symphony {
namespace internal {
#define SYMPHONY_HAS_ATOMIC_DMWORD 1
#if SYMPHONY_SIZEOF_MWORD == 8
  typedef unsigned __int128 symphony_dmword_t;
#else
  typedef uint64_t symphony_dmword_t;
#endif
};
};
#elif defined(__GNUC__)
#if __LP64__
#define SYMPHONY_HAS_ATOMIC_DMWORD __GLIBCXX_USE_INT128
#else
#define SYMPHONY_HAS_ATOMIC_DMWORD 1
#endif
namespace symphony {
namespace internal {
#if __GLIBCXX_USE_INT128
  typedef unsigned __int128 symphony_dmword_t;
#else
  typedef uint64_t symphony_dmword_t;
#endif
};
};
#elif defined(_M_IX86) || defined(_M_X64)
#define SYMPHONY_HAS_ATOMIC_DMWORD 1
namespace symphony {
namespace internal {

typedef ULONGLONG symphony_dmword_t;
};
};
#else
#error No implementation for atomic operations available for this architecture.
#endif
