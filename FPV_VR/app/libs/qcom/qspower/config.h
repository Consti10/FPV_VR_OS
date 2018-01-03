/* Define to 1 if you have the <android/log.h> header file. */
#define HAVE_ANDROID_LOG_H 1

/* Define to 1 if you have the <cxxabi.h> header file. */
#define HAVE_CXXABI_H 1

/* Define to 1 if you have C++ std::atomic support. */
#define HAVE_CXX_STD_ATOMIC 1

/* Define to 1 if you have C++ std::thread support. */
#define HAVE_CXX_STD_THREAD 1

/* Define the version of Power Optimization SDK */
#define QSPOWER_VERSION "1.0.0"

/* Define to 1 if pthread_t can be cast as integer type. */
#define HAVE_INTEGRAL_PTHREAD_SELF 1

/* Define to 1 if you have the `__android_log_vprint' function. */
#define HAVE___ANDROID_LOG_VPRINT 1

/* Have __builtin_popcount. */
#define HAVE___BUILTIN_POPCOUNT 1

/* Have __builtin_popcountl. */
#define HAVE___BUILTIN_POPCOUNTL 1

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
    #define inline inline
    #define POWER_INLINE inline
#else
// for C++, inline is a keyword
    #define POWER_INLINE inline
#endif
