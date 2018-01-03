// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

#include <symphony/internal/compat/compilercompat.h>

#ifndef SYMPHONY_INLINE
#ifdef _MSC_VER
#define SYMPHONY_INLINE __inline
#else
#define SYMPHONY_INLINE inline
#endif
#endif

static SYMPHONY_INLINE int
symphony_popcount(uint32_t x)
{
#ifdef HAVE___BUILTIN_POPCOUNT
  return __builtin_popcount(x);
#else
#define SYMPHONY_INIT1(X)                                                        \
  ((((X) & (1 << 0)) != 0) +                                            \
   (((X) & (1 << 1)) != 0) +                                            \
   (((X) & (1 << 2)) != 0) +                                            \
   (((X) & (1 << 3)) != 0) +                                            \
   (((X) & (1 << 4)) != 0) +                                            \
   (((X) & (1 << 5)) != 0) +                                            \
   (((X) & (1 << 6)) != 0) +                                            \
   (((X) & (1 << 7)) != 0))
#define SYMPHONY_INIT2(X)   SYMPHONY_INIT1(X),  SYMPHONY_INIT1((X) +  1)
#define SYMPHONY_INIT4(X)   SYMPHONY_INIT2(X),  SYMPHONY_INIT2((X) +  2)
#define SYMPHONY_INIT8(X)   SYMPHONY_INIT4(X),  SYMPHONY_INIT4((X) +  4)
#define SYMPHONY_INIT16(X)  SYMPHONY_INIT8(X),  SYMPHONY_INIT8((X) +  8)
#define SYMPHONY_INIT32(X) SYMPHONY_INIT16(X), SYMPHONY_INIT16((X) + 16)
#define SYMPHONY_INIT64(X) SYMPHONY_INIT32(X), SYMPHONY_INIT32((X) + 32)

  static const uint8_t popcount8[256] = {
  SYMPHONY_INIT64(0), SYMPHONY_INIT64(64), SYMPHONY_INIT64(128), SYMPHONY_INIT64(192)
    };

  return (popcount8[x & 0xff] +
          popcount8[(x >> 8) & 0xff] +
          popcount8[(x >> 16) & 0xff] +
          popcount8[x >> 24]);
#endif
          }

SYMPHONY_CLANG_IGNORE_BEGIN("-Wold-style-cast");

 static SYMPHONY_INLINE int
 symphony_popcountl(unsigned long int x)
 {
#ifdef HAVE___BUILTIN_POPCOUNTL
   return __builtin_popcountl(x);
#else
   int cnt = symphony_popcount((uint32_t)(x));
   if (sizeof x > 4)
     cnt += symphony_popcount((uint32_t)(x >> 31 >> 1));
   return cnt;
#endif
 }

static SYMPHONY_INLINE int
symphony_popcountw(size_t x)
{
  int cnt = symphony_popcount((uint32_t)(x));
  if (sizeof x > 4)
    cnt += symphony_popcount((uint32_t)(x >> 31 >> 1));
  return cnt;
}

SYMPHONY_CLANG_IGNORE_END("-Wold-style-cast");

#ifdef __cplusplus
}
#endif
