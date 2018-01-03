// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef __GNUC__
#define SYMPHONY_GCC_PRAGMA(x) _Pragma(#x)
#define SYMPHONY_GCC_PRAGMA_DIAG(x) SYMPHONY_GCC_PRAGMA(GCC diagnostic x)
#define SYMPHONY_GCC_IGNORE_BEGIN(x) SYMPHONY_GCC_PRAGMA_DIAG(push)             \
  SYMPHONY_GCC_PRAGMA_DIAG(ignored x)
#define SYMPHONY_GCC_IGNORE_END(x) SYMPHONY_GCC_PRAGMA_DIAG(pop)
#ifdef __clang__
#define SYMPHONY_CLANG_PRAGMA_DIAG(x) SYMPHONY_GCC_PRAGMA(clang diagnostic x)
#define SYMPHONY_CLANG_IGNORE_BEGIN(x) SYMPHONY_CLANG_PRAGMA_DIAG(push)         \
  SYMPHONY_CLANG_PRAGMA_DIAG(ignored x)
#define SYMPHONY_CLANG_IGNORE_END(x) SYMPHONY_CLANG_PRAGMA_DIAG(pop)
#else
#define SYMPHONY_CLANG_IGNORE_BEGIN(x)
#define SYMPHONY_CLANG_IGNORE_END(x)
#endif
#else
#define SYMPHONY_GCC_IGNORE_BEGIN(x)
#define SYMPHONY_GCC_IGNORE_END(x)
#define SYMPHONY_CLANG_IGNORE_BEGIN(x)
#define SYMPHONY_CLANG_IGNORE_END(x)
#endif

#ifdef _MSC_VER
#define SYMPHONY_MSC_PRAGMA(x) __pragma(x)
#define SYMPHONY_MSC_PRAGMA_WARNING(x) SYMPHONY_MSC_PRAGMA(warning(x))
#define SYMPHONY_MSC_IGNORE_BEGIN(x) SYMPHONY_MSC_PRAGMA_WARNING(push)          \
  SYMPHONY_MSC_PRAGMA_WARNING(disable : x)
#define SYMPHONY_MSC_IGNORE_END(x) SYMPHONY_MSC_PRAGMA_WARNING(pop)
#else
#define SYMPHONY_MSC_IGNORE_BEGIN(x)
#define SYMPHONY_MSC_IGNORE_END(x)
#endif

#if defined(__clang__)

  #define SYMPHONY_PUBLIC __attribute__ ((visibility ("default")))
  #define SYMPHONY_PUBLIC_START  SYMPHONY_GCC_PRAGMA(GCC visibility push(default))
  #define SYMPHONY_PUBLIC_STOP   SYMPHONY_GCC_PRAGMA(GCC visibility pop)
  #define SYMPHONY_PRIVATE __attribute__ ((visibility ("hidden")))
  #define SYMPHONY_PRIVATE_START SYMPHONY_GCC_PRAGMA(GCC visibility push(hidden))
  #define SYMPHONY_PRIVATE_STOP  SYMPHONY_GCC_PRAGMA(GCC visibility pop)
#elif defined(__GNUC__)
  #define SYMPHONY_PUBLIC  __attribute__ ((visibility ("default")))
  #define SYMPHONY_PUBLIC_START  SYMPHONY_GCC_PRAGMA(GCC visibility push(default))
  #define SYMPHONY_PUBLIC_STOP   SYMPHONY_GCC_PRAGMA(GCC visibility pop)
  #define SYMPHONY_PRIVATE __attribute__ ((visibility ("hidden")))
  #define SYMPHONY_PRIVATE_START SYMPHONY_GCC_PRAGMA(GCC visibility push(hidden))
  #define SYMPHONY_PRIVATE_STOP  SYMPHONY_GCC_PRAGMA(GCC visibility pop)
#elif defined(_MSC_VER)
  #ifdef SYMPHONY_DLL_EXPORT

    #define SYMPHONY_PUBLIC __declspec(dllexport)
  #else

    #define SYMPHONY_PUBLIC __declspec(dllimport)
  #endif
  #define SYMPHONY_PRIVATE
#else
  #error "Unknown compiler. Please use GCC, Clang, or Visual Studio!"
#endif
