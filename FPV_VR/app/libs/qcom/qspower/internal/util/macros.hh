#pragma once

// Macro to suppress warnings about unused variables
#define QSPOWER_UNUSED(x) ((void)x)

// Macros to make the VS release numbers more memorable
#ifdef _MSC_VER
    #define QSPOWER_VS2015 1900
    #define QSPOWER_VS2013 1800
    #define QSPOWER_VS2012 1700
#endif // _MSC_VER

// Macro to define the proper incantation of inline for C files
#ifndef QSPOWER_INLINE
    #ifdef _MSC_VER
        #define QSPOWER_INLINE __inline
    #else
        #define QSPOWER_INLINE inline
    #endif // _MSC_VER
#endif

// Macro to define the proper constexpr for different compilers
#ifndef QSPOWER_CONSTEXPR
    #ifdef _MSC_VER
        #define QSPOWER_CONSTEXPR
    #else
        #define QSPOWER_CONSTEXPR constexpr
    #endif // _MSC_VER
#endif // QSPOWER_CONSTEXPR

// and the replacement with const
#ifndef QSPOWER_CONSTEXPR_CONST
    #ifdef _MSC_VER
        #define QSPOWER_CONSTEXPR_CONST const
    #else
        #define QSPOWER_CONSTEXPR_CONST constexpr
    #endif // _MSC_VER
#endif // QSPOWER_CONSTEXPR_CONST

// Macros to implement the alignment stuff
#ifdef _MSC_VER
    #define QSPOWER_ALIGN(size) __declspec(align(size))
    #define QSPOWER_ALIGNED(type, name, size) __declspec(align(size)) type name
#else
    #define QSPOWER_ALIGN(size) __attribute__((aligned(size)))
    #define QSPOWER_ALIGNED(type, name, size) type name __attribute__((aligned(size)))
#endif

// Macro to provide an interface for GCC-specific attributes, but that
// can be skipped on different compilers such as VS2012 that do not
// support these.
#ifdef _MSC_VER
    #define QSPOWER_GCC_ATTRIBUTE(args)
#else
    #define QSPOWER_GCC_ATTRIBUTE(args) __attribute__(args)
#endif

#ifdef __GNUC__
    #define QSPOWER_DEPRECATED(decl) decl __attribute__((deprecated))
#elif defined(_MSC_VER)
    #define QSPOWER_DEPRECATED(decl) __declspec(deprecated) decl
#else
    #warning No QSPOWER_DEPRECATED implementation for this compiler
    #define QSPOWER_DEPRECATED(decl) decl
#endif

// VS2012 does not support the noexcept keyword, so use a macro to
// provide different implementations. VS2015 does.
#if defined(_MSC_VER) && (_MSC_VER < QSPOWER_VS2015)
    #define QSPOWER_NOEXCEPT
#else
    #define QSPOWER_NOEXCEPT noexcept
#endif

// VS2012 does not support explicitly deleted methods, so use a macro
// to provide different implementations. VS2015 does.
#if defined(_MSC_VER) && (_MSC_VER < QSPOWER_VS2015)
    // We do not provide an implementation here. This is ok, and if
    // someone tries to call it, they will get a linker error.  You cannot
    // put an implementation here, because some constructors need to
    // specify initializers.
    #define QSPOWER_DELETE_METHOD(...) __VA_ARGS__
#else
    // Use C++11 features to do this more cleanly
    #define QSPOWER_DELETE_METHOD(...) __VA_ARGS__ = delete
#endif

// VS2012 does not support explicit default constructors, so use a
// macro to provide different implementations. VS2015 does.
#if defined(_MSC_VER) && (_MSC_VER < QSPOWER_VS2015)
    #define QSPOWER_DEFAULT_METHOD(...) __VA_ARGS__ {}
#else
    #define QSPOWER_DEFAULT_METHOD(...) __VA_ARGS__ = default
#endif
