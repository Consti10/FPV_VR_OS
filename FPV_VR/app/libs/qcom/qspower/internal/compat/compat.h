#pragma once

#include <sys/stat.h>

// define a compatible version of the stat() system call
#ifdef _MSC_VER
    #define QSPOWER_STAT _stat
#else
    #define QSPOWER_STAT stat
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifdef _MSC_VER
    // Include Windows headers, disable min() and max() macros
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif // NOMINMAX

    #include <Windows.h>
    #include <errno.h>

    // Provide the equivalent GCC intrinsic for VS2012
    static QSPOWER_INLINE void
    __sync_synchronize(void)
    {
        MemoryBarrier();
    }
#endif // _MSC_VER

#ifdef _MSC_VER
    // Implement ssize_t and pid_t for VS2012
    typedef SSIZE_T ssize_t;
    typedef int pid_t;
#else
    #include <inttypes.h>
    #include <sys/types.h>
#endif // _MSC_VER

#if defined(__ANDROID__) || defined(__linux__)
    #include <errno.h>
    #include <pthread.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <sys/utsname.h>
    #include <unistd.h>
#endif

#ifndef _MSC_VER
    #include <string.h>
#endif

#if defined(__ANDROID__)
    #include <qspower/internal/compat/compiler_compat.h>

    // ignore warning of static int stat() shadowing struct stat in Android headers
    QSPOWER_GCC_IGNORE_BEGIN("-Wshadow")
    #include <sys/stat.h>
    QSPOWER_GCC_IGNORE_END("-Wshadow")
#elif defined(__linux__)
    #include <sched.h>
#else
    // Currently we only provide support for pinning threads on Android and Linux.
#endif

#include <qspower/internal/util/macros.hh>

uint64_t
qspower_get_time_now();

#ifdef _MSC_VER
    __declspec(noreturn) void
    qspower_exit(int);
#else
    void
    qspower_exit(int) QSPOWER_GCC_ATTRIBUTE ((noreturn));
#endif // _MSC_VER

void
qspower_android_logprintf(int level, const char *format, ...) QSPOWER_GCC_ATTRIBUTE ((format (printf, 2, 3)));

#ifdef __cplusplus
    // Check there are no min or max macros defined which interfere with
    // our std:: usage
    #ifdef min
        #ifdef _MSC_VER
        static_assert(false, "Cannot use std::min when there is an existing min "
                             "#define, on VS2012 you should define NOMINMAX when "
                             "including Windows.h");
        #else
            #error Cannot use std::min when there is an existing min #define
        #endif // _MSC_VER
    #endif // min

    #ifdef max
        #ifdef _MSC_VER
            static_assert(false, "Cannot use std::max when there is an existing min "
                                 "#define, on VS2012 you should define NOMINMAX "
                                 "when including Windows.h");
        #else
            #error Cannot use std::max when there is an existing max #define
        #endif // _MSC_VER
    #endif // max
#endif // __cplusplus

/// static_assert(constexpr,...) does not work on VS2012, because it
/// does not support constexpr. So use a macro to evaluate at runtime.
#ifdef _MSC_VER
    #define qspower_constexpr_static_assert(cond, format, ...) \
        do { if (!cond) QSPOWER_FATAL(format, ## __VA_ARGS__); } while (0)
#else
    #define qspower_constexpr_static_assert static_assert
#endif

#ifdef _MSC_VER
    // VS2012 does not provide unistd.h and usleep() or sleep(), so
    // provide a replacement
    #ifdef __cplusplus
        static inline void usleep(size_t ms) { Sleep(static_cast<DWORD>(ms) / 1000); }
        static inline void sleep(size_t s) { Sleep(static_cast<DWORD>(s) * 1000); }
    #else
        static void usleep(size_t ms) { Sleep((DWORD)ms / 1000); }
        static void sleep(size_t s) { Sleep((DWORD)s * 1000); }
    #endif // __cplusplus
#else
    #include <unistd.h>
#endif // _MSC_VER

#ifdef _MSC_VER
    // Provide a replacement for pthread TLS support on VS2012
    typedef DWORD pthread_key_t;

    ///////////////////////////////////////////////////////////////////////////////

    static QSPOWER_INLINE
    int pthread_key_create(pthread_key_t *key, void (*destructor)(void*))
    {
        QSPOWER_UNUSED(destructor);

        *key = TlsAlloc();
        if (*key == TLS_OUT_OF_INDEXES) { return 1; }
        else { return 0; }
    }

    ///////////////////////////////////////////////////////////////////////////////

    static QSPOWER_INLINE
    int pthread_key_delete(pthread_key_t key)
    {
        // TlsFree returns 0 on failure, but pthread_key_delete is 0 on success
        return !TlsFree(key);
    }

    ///////////////////////////////////////////////////////////////////////////////

    static QSPOWER_INLINE
    void* pthread_getspecific(pthread_key_t key)
    {
        return TlsGetValue(key);
    }

    ///////////////////////////////////////////////////////////////////////////////

    static QSPOWER_INLINE
    int pthread_setspecific(pthread_key_t key, const void *value)
    {
        // TlsSetValue returns 0 on failure, but pthread_setspecific is 0 on success
        return !TlsSetValue(key, (LPVOID)value);
    }
#endif // _MSC_VER

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus
    #include <string>

    #ifdef _MSC_VER
        static inline std::string
        qspower_strerror(int errnum_)
        {
            char buf[1024] = "";
            strerror_s(buf, errnum_);
            return buf;
        }
    #elif defined(__APPLE__) || defined(__ANDROID__) || \
          (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! defined _GNU_SOURCE
        #include <cstring>

        static inline std::string
        qspower_strerror(int errnum_)
        {
            char buf[1024] = "";
            ::strerror_r(errnum_, buf, sizeof buf);
            return buf;
        }
    #else
        #include <cstring>

        static inline std::string
        qspower_strerror(int errnum_)
        {
            char buf[1024] = "";
            return ::strerror_r(errnum_, buf, sizeof buf);
        }
    #endif // _MSC_VER

    ///////////////////////////////////////////////////////////////////////////////

    static inline size_t qspower_getpagesize()
    {
        static size_t _ps = 0;
        if (_ps == 0)
        {
        #ifdef _MSC_VER
            _ps = 4096;
        #else
            _ps = sysconf(_SC_PAGESIZE);
        #endif

            if(_ps <= 1) { _ps = 4096; } // some error, we set it to 4K
        }

        return _ps;
    }
#endif // __cplusplus

/**
 * @brief Copies @c src to @c dst
 * Up to @c max-1 characters are copied, then NUL is appended.
 * @note @c dst and @c src should NOT 'overlap'
 * @param dst The destionation
 * @param src The source
 * @param max The maximum number of characters @c dst can contain, including NUL
 * @return -1: Error
 *         [0, max): The number of characters written (excluding NUL)
 *         max: dst not big enough
 */
#ifdef __cplusplus
extern "C"
#endif
int
qspower_string_copy(char* dst, const char* src, int max);
