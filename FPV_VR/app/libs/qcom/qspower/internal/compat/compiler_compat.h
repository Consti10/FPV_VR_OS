#pragma once
// Define a set macros that are used for compilers compatibility

#ifdef __GNUC__
    #define QSPOWER_GCC_PRAGMA(x) _Pragma(#x)
    #define QSPOWER_GCC_PRAGMA_DIAG(x) QSPOWER_GCC_PRAGMA(GCC diagnostic x)
    #define QSPOWER_GCC_IGNORE_BEGIN(x) QSPOWER_GCC_PRAGMA_DIAG(push)         \
                                        QSPOWER_GCC_PRAGMA_DIAG(ignored x)
    #define QSPOWER_GCC_IGNORE_END(x) QSPOWER_GCC_PRAGMA_DIAG(pop)

    #ifdef __clang__
        #define QSPOWER_CLANG_PRAGMA_DIAG(x) QSPOWER_GCC_PRAGMA(clang diagnostic x)
        #define QSPOWER_CLANG_IGNORE_BEGIN(x) QSPOWER_CLANG_PRAGMA_DIAG(push)         \
                                              QSPOWER_CLANG_PRAGMA_DIAG(ignored x)
        #define QSPOWER_CLANG_IGNORE_END(x) QSPOWER_CLANG_PRAGMA_DIAG(pop)
    #else
        #define QSPOWER_CLANG_IGNORE_BEGIN(x)
        #define QSPOWER_CLANG_IGNORE_END(x)
    #endif // __clang__
#else
    #define QSPOWER_GCC_IGNORE_BEGIN(x)
    #define QSPOWER_GCC_IGNORE_END(x)
    #define QSPOWER_CLANG_IGNORE_BEGIN(x)
    #define QSPOWER_CLANG_IGNORE_END(x)
#endif // __GNUC__

#ifdef _MSC_VER
    #define QSPOWER_MSC_PRAGMA(x) __pragma(x)
    #define QSPOWER_MSC_PRAGMA_WARNING(x) QSPOWER_MSC_PRAGMA(warning(x))
    #define QSPOWER_MSC_IGNORE_BEGIN(x) QSPOWER_MSC_PRAGMA_WARNING(push)          \
                                        QSPOWER_MSC_PRAGMA_WARNING(disable : x)
    #define QSPOWER_MSC_IGNORE_END(x) QSPOWER_MSC_PRAGMA_WARNING(pop)
#else
    #define QSPOWER_MSC_IGNORE_BEGIN(x)
    #define QSPOWER_MSC_IGNORE_END(x)
#endif

// Macros to control symbol visibility
#if defined(__clang__) /* clang must be first since it also defines __GNUC__*/
    // for now, it defines the same rules as GCC
    #define QSPOWER_PUBLIC __attribute__ ((visibility ("default")))
    #define QSPOWER_PUBLIC_START  QSPOWER_GCC_PRAGMA(GCC visibility push(default))
    #define QSPOWER_PUBLIC_STOP   QSPOWER_GCC_PRAGMA(GCC visibility pop)
    #define QSPOWER_PRIVATE __attribute__ ((visibility ("hidden")))
    #define QSPOWER_PRIVATE_START QSPOWER_GCC_PRAGMA(GCC visibility push(hidden))
    #define QSPOWER_PRIVATE_STOP  QSPOWER_GCC_PRAGMA(GCC visibility pop)
#elif defined(__GNUC__)
    #define QSPOWER_PUBLIC  __attribute__ ((visibility ("default")))
    #define QSPOWER_PUBLIC_START  QSPOWER_GCC_PRAGMA(GCC visibility push(default))
    #define QSPOWER_PUBLIC_STOP   QSPOWER_GCC_PRAGMA(GCC visibility pop)
    #define QSPOWER_PRIVATE __attribute__ ((visibility ("hidden")))
    #define QSPOWER_PRIVATE_START QSPOWER_GCC_PRAGMA(GCC visibility push(hidden))
    #define QSPOWER_PRIVATE_STOP  QSPOWER_GCC_PRAGMA(GCC visibility pop)
#elif defined(_MSC_VER)
    #ifdef QSPOWER_DLL_EXPORT
        /* We are building the qspower library */
        #define QSPOWER_PUBLIC __declspec(dllexport)
    #else
        /* We are using this library */
        #define QSPOWER_PUBLIC __declspec(dllimport)
    #endif // QSPOWER_DLL_EXPORT
    #define QSPOWER_PRIVATE
#else
    #error "Unknown compiler. Please use GCC, Clang, or Visual Studio."
#endif
