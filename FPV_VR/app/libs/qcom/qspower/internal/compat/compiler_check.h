#pragma once

// A set of compiler version checks so that we can fail quickly without
// poluting all the code with compiler versions.
#if defined(__ANDROID__)
    #if defined(__clang__)
        // CLANG defines both __clang__ and __GNUC__ so it needs a separate check
        #if (__clang_major__ < 3) || (__clang_major == 3 && __clang_minor__ < 4)
            #error "Power Optimization SDK requires Android LLVM 3.4 or higher"
        #endif
    #elif defined (__GNUC__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 8))
        #error "Power Optimization SDK requires Android GNU C++ 4.8 or higher"
    #endif
#elif defined(__APPLE__)
    #if defined(__clang__)
        // __apple_build_version__ is only defined for Apple LLVM
        #if defined(__apple_build_version__)
            // CLANG defines both __clang__ and __GNUC__ so it needs a separate check
            // Apple LLVM version 5.1 was the first Apple LLVM based on LLVM version 3.4
            #if (__clang_major__ < 5) || (__clang_major__ == 5 && __clang_minor__ < 1)
                #error "Power Optimization SDK requires Apple Clang 5.1 or higher"
            #endif
        #else
            #if (__clang_major__ < 3) || (__clang_major == 3 && __clang_minor__ < 4)
                #error "Power Optimization SDK requires LLVM 3.4 or higher in mac os x"
            #endif
        #endif
    #elif defined (__GNUC__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
        #error "Power Optimization SDK requires GNU C++ 4.7 or higher"
    #endif
#elif defined (_MSC_FULL_VER)
    #if (_MSC_VER < 1900)
        #error "Power Optimization SDK requires Visual Studio 2015 or higher"
    #endif
#elif defined(__linux__)
    #if defined(__clang__)
        // CLANG defines both __clang__ and __GNUC__ so it needs a separate check
        #if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 3)
            #error "Power Optimization SDK requires LLVM 3.3 or higher"
        #endif
    #elif defined (__GNUC__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
        #error "Power Optimization SDK requires GNU C++ 4.7 or higher"
    #endif
#else
    #error "Platform not supported!"
#endif
