// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/util/exceptions.hh>
#include <symphony/internal/util/memorder.hh>

#ifdef __ANDROID__

SYMPHONY_GCC_IGNORE_BEGIN("-Wshadow")
SYMPHONY_GCC_IGNORE_BEGIN("-Wold-style-cast")
#endif
#include <atomic>
#include <cstring>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <symphony/internal/compat/compat.h>
#include <symphony/internal/util/platform_tid.h>

#ifdef __ANDROID__
SYMPHONY_GCC_IGNORE_END("-Wold-style-cast")
SYMPHONY_GCC_IGNORE_END("-Wshadow")
#endif

#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/tls.hh>

#ifndef _MSC_VER

extern "C" void symphony_breakpoint() __attribute__((noinline));
#else
SYMPHONY_INLINE void symphony_breakpoint() {}
#endif

namespace symphony {

namespace internal {

using ::symphony_exit;
using ::symphony_android_logprintf;

extern std::atomic<unsigned int> symphony_log_counter;

};

};

#ifdef _MSC_VER

#include <string>
inline const std::string symphony_c99_vsformat (const char* _format)
{
  bool last_pct = false;
  std::string fmtcopy (_format);
  for (auto& i : fmtcopy) {
    if (i == '%') {

      last_pct = !last_pct;
    } else {
      if (last_pct) {
        if (i == 'z') {

          i = 'I';
          last_pct = false;
        } else if (isdigit(i)) {

        } else {

          last_pct = false;
        }
      }
    }
  }
  return fmtcopy;
}
#endif

#ifdef __ANDROID__

#ifdef __aarch64__
#define SYMPHONY_FMT_TID "lx"
#else
#define SYMPHONY_FMT_TID "x"
#endif
#define  SYMPHONY_DLOG(format, ...)                                               \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_DEBUG,                \
      "\033[32mD %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m",             \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed), \
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_DLOGN(format, ...)                                              \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_DEBUG,                \
      "\033[32mD %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m",             \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed),\
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_ILOG(format, ...)                                               \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_INFO,                 \
      "\033[33mI %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m",             \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed),\
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_WLOG(format, ...)                                               \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_WARN,                 \
      "\033[35mW %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m",             \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed),\
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_EXIT_FATAL(format, ...)                                         \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_ERROR,                \
      "\033[31mFATAL %8x t%" SYMPHONY_FMT_TID " %s:%d %s() " format,              \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed),\
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__, __FUNCTION__ ,   \
      ## __VA_ARGS__),                                                        \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_ERROR,                \
      "t%" SYMPHONY_FMT_TID " %s:%d **********",                                  \
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__),                 \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_ERROR,                \
      "t%" SYMPHONY_FMT_TID " %s:%d - Terminating with exit(1)",                  \
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__),                 \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_ERROR,                \
      "t%" SYMPHONY_FMT_TID " %s:%d **********\033[0m",                           \
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__),                 \
  symphony_exit(1)
#define  SYMPHONY_ALOG(format, ...)                                               \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_INFO,                 \
      "\033[36mA %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m",             \
      ::symphony::internal::symphony_log_counter.fetch_add(1,::symphony::mem_order_relaxed),\
      symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_LLOG(format, ...)                                               \
  ::symphony::internal::symphony_android_logprintf (ANDROID_LOG_INFO, format "\n",    \
                                            ## __VA_ARGS__)
#else

#if defined(PRIxPTR)
#define SYMPHONY_FMT_TID PRIxPTR
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__ARM_ARCH_7A__)
#define SYMPHONY_FMT_TID "x"
#else
#define SYMPHONY_FMT_TID "lx"
#endif
#ifdef _MSC_VER

#define SYMPHONY_LOG_FPRINTF(stream, fmt, ...) fprintf(stream, symphony_c99_vsformat(fmt).c_str(), ## __VA_ARGS__)
#else
#define SYMPHONY_LOG_FPRINTF(stream, fmt, ...) fprintf(stream, fmt, ## __VA_ARGS__)
#endif
#define  SYMPHONY_DLOG(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[32mD %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m\n", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_DLOGN(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[32mD %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_ILOG(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[33mI %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m\n", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_WLOG(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[35mW %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m\n", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_EXIT_FATAL(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[31mFATAL %8x t%" SYMPHONY_FMT_TID " %s:%d %s() " format "\n", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__, __FUNCTION__ , ## __VA_ARGS__), SYMPHONY_LOG_FPRINTF(stderr,"t%" SYMPHONY_FMT_TID " %s:%d - Terminating with exit(1)\033[0m\n", symphony_internal_get_platform_thread_id(), __FILE__, __LINE__), symphony_exit(1)
#define  SYMPHONY_ALOG(format, ...) SYMPHONY_LOG_FPRINTF(stderr,"\033[36mA %8x t%" SYMPHONY_FMT_TID " %s:%d " format "\033[0m\n", symphony::internal::symphony_log_counter.fetch_add(1,symphony::mem_order_relaxed), symphony_internal_get_platform_thread_id(), __FILE__, __LINE__ , ## __VA_ARGS__)
#define  SYMPHONY_LLOG(format, ...) SYMPHONY_LOG_FPRINTF(stderr, format "\n", ## __VA_ARGS__)
#endif

#define SYMPHONY_UNIMPLEMENTED(format, ...)  SYMPHONY_EXIT_FATAL("Unimplemented code in function '%s' at %s:%d " format, __FUNCTION__, __FILE__, __LINE__, ## __VA_ARGS__)
#define SYMPHONY_UNREACHABLE(format, ...)  SYMPHONY_EXIT_FATAL("Unreachable code triggered in function '%s' at %s:%d " format, __FUNCTION__, __FILE__, __LINE__, ## __VA_ARGS__)

#ifndef SYMPHONY_THROW_ON_API_ASSERT
#define SYMPHONY_FATAL(format, ...) SYMPHONY_EXIT_FATAL(format, ## __VA_ARGS__)
#else
#define SYMPHONY_FATAL(format, ...) do {                                    \
    throw ::symphony::internal::fatal_exception(::symphony::internal::          \
                              strprintf(format, ## __VA_ARGS__),        \
                              __FILE__, __LINE__, __FUNCTION__);        \
    } while(false)
#endif

#ifdef  SYMPHONY_THROW_ON_API_ASSERT
#define SYMPHONY_API_ASSERT(cond, format, ...) do {                         \
    if(!(cond)) { throw                                                 \
        ::symphony::internal::api_assert_exception(::symphony::internal::       \
                              strprintf(format, ## __VA_ARGS__),        \
                              __FILE__, __LINE__, __FUNCTION__);        \
    } } while(false)
#else
#define SYMPHONY_API_ASSERT_COND(cond) #cond
#define SYMPHONY_API_ASSERT(cond, format, ...) do { if(!(cond)) { \
      SYMPHONY_EXIT_FATAL("API assert [%s] - " format, SYMPHONY_API_ASSERT_COND(cond), ## __VA_ARGS__); } \
  } while(false)
#endif

#ifdef SYMPHONY_DEBUG
#define SYMPHONY_CHECK_INTERNAL
#endif

#ifdef SYMPHONY_DISABLE_CHECK_INTERNAL
#undef SYMPHONY_CHECK_INTERNAL
#endif

#ifdef SYMPHONY_ENABLE_CHECK_INTERNAL
#ifndef SYMPHONY_CHECK_INTERNAL
#define SYMPHONY_CHECK_INTERNAL
#endif
#endif

#ifdef SYMPHONY_CHECK_INTERNAL
#ifndef SYMPHONY_THROW_ON_API_ASSERT
#define SYMPHONY_INTERNAL_ASSERT_COND(cond) #cond
#define SYMPHONY_INTERNAL_ASSERT(cond, format, ...) do { if(!(cond)) {      \
      SYMPHONY_EXIT_FATAL("Internal assert failed [%s] - " format, SYMPHONY_INTERNAL_ASSERT_COND(cond), ## __VA_ARGS__); } \
  } while(false)
#else
#define SYMPHONY_INTERNAL_ASSERT(cond, format, ...) do {                    \
    if(!(cond)) { throw                                                 \
        ::symphony::internal::assert_exception(::symphony::internal::           \
                              strprintf(format, ## __VA_ARGS__),        \
                              __FILE__, __LINE__, __FUNCTION__);        \
    } } while(false)
#endif
#else
#define SYMPHONY_INTERNAL_ASSERT(cond, format, ...) do {                    \
      if(false) { if(cond){} if(format){} } } while(false)
#endif

#ifndef SYMPHONY_DEBUG

#ifndef SYMPHONY_NOLOG
# define SYMPHONY_NOLOG
#endif
#endif

#ifdef SYMPHONY_NOLOG
# undef  SYMPHONY_DLOG
# undef  SYMPHONY_DLOGN
# undef  SYMPHONY_ILOG
# undef  SYMPHONY_WLOG
# define  SYMPHONY_DLOG(format, ...) do {} while (false)
# define  SYMPHONY_DLOGN(format, ...) do {} while (false)
# define  SYMPHONY_ILOG(format, ...) do {} while (false)
# define  SYMPHONY_WLOG(format, ...) do {} while (false)
#endif
