// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#define SYMPHONY_UNUSED(x) ((void)x)

#ifdef _MSC_VER
#define SYMPHONY_VS2015 1900
#define SYMPHONY_VS2013 1800
#define SYMPHONY_VS2012 1700
#endif

#ifndef SYMPHONY_INLINE
#ifdef _MSC_VER
#define SYMPHONY_INLINE __inline
#else
#define SYMPHONY_INLINE inline
#endif
#endif

#ifndef SYMPHONY_CONSTEXPR
#ifdef _MSC_VER
#define SYMPHONY_CONSTEXPR
#else
#define SYMPHONY_CONSTEXPR constexpr
#endif
#endif

#ifndef SYMPHONY_CONSTEXPR_CONST
#ifdef _MSC_VER
#define SYMPHONY_CONSTEXPR_CONST const
#else
#define SYMPHONY_CONSTEXPR_CONST constexpr
#endif
#endif

#ifdef _MSC_VER

#define SYMPHONY_ALIGN(size) __declspec(align(size))
#define SYMPHONY_ALIGNED(type, name, size) \
  __declspec(align(size)) type name

#else

#define SYMPHONY_ALIGN(size) __attribute__ ((aligned(size)))
#define SYMPHONY_ALIGNED(type, name, size) \
  type name __attribute__ ((aligned(size)))

#endif

#ifdef _MSC_VER
#define SYMPHONY_GCC_ATTRIBUTE(args)
#else
#define SYMPHONY_GCC_ATTRIBUTE(args) __attribute__(args)
#endif

#ifdef __GNUC__
#define SYMPHONY_DEPRECATED(decl) decl __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SYMPHONY_DEPRECATED(decl) __declspec(deprecated) decl
#else
#warning No SYMPHONY_DEPRECATED implementation for this compiler
#define SYMPHONY_DEPRECATED(decl) decl
#endif

#if defined(_MSC_VER) && (_MSC_VER < SYMPHONY_VS2015)
#define SYMPHONY_NOEXCEPT
#else
#define SYMPHONY_NOEXCEPT noexcept
#endif

#if defined(_MSC_VER) && (_MSC_VER < SYMPHONY_VS2015)

#define SYMPHONY_DELETE_METHOD(...) __VA_ARGS__
#else

#define SYMPHONY_DELETE_METHOD(...) __VA_ARGS__ = delete
#endif

#if defined(_MSC_VER) && (_MSC_VER < SYMPHONY_VS2015)
#define SYMPHONY_DEFAULT_METHOD(...) __VA_ARGS__ { }
#else
#define SYMPHONY_DEFAULT_METHOD(...) __VA_ARGS__ = default
#endif
