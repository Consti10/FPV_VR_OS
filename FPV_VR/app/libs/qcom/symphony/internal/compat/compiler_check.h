// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if defined(__ANDROID__)
  #if defined(__clang__)

    #if (__clang_major__ < 3) || (__clang_major == 3 && __clang_minor__ < 4)
      #error "Symphony requires Android LLVM 3.4 or higher"
    #endif
  #elif defined (__GNUC__) && ((__GNUC__ < 4) ||                        \
                               (__GNUC__ == 4 && __GNUC_MINOR__ < 8))
    #error "Symphony requires Android GNU C++ 4.8 or higher"
  #endif
#elif defined(__APPLE__)
  #if defined(__clang__)

    #if defined(__apple_build_version__)

      #if (__clang_major__ < 5) || (__clang_major__ == 5 && __clang_minor__ < 1)
         #error "Symphony requires Apple Clang 5.1 or higher"
      #endif
    #else
      #if (__clang_major__ < 3) || (__clang_major == 3 && __clang_minor__ < 4)
        #error "Symphony requires LLVM 3.4 or higher in mac os x"
      #endif
    #endif
  #elif defined (__GNUC__) && ((__GNUC__ < 4) ||                        \
                               (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
    #error "Symphony requires GNU C++ 4.7 or higher"
  #endif
#elif defined (_MSC_FULL_VER)
  #if (_MSC_VER < 1900)
    #error "Symphony requires Visual Studio 2015 or higher"
  #endif
#elif defined(__linux__)
  #if defined(__clang__)

    #if (__clang_major__ < 3) || (__clang_major__ == 3 && __clang_minor__ < 3)
      #error "Symphony requires LLVM 3.3 or higher"
    #endif
  #elif defined (__GNUC__) && ((__GNUC__ < 4) ||                        \
                               (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
    #error "Symphony requires GNU C++ 4.7 or higher"
  #endif
#else
  #error "Platform not supported!"
#endif
