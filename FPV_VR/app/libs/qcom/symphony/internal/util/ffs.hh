// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <stdint.h>

namespace symphony {
namespace internal {

inline int
symphony_ffs(uint32_t x)
{
  if (x == 0)
    return 0;

#ifdef _MSC_VER
  unsigned long index;
  if (_BitScanForward(&index, x))
    return index + 1;
  return 0;
#else
  return __builtin_ffs(x);
#endif
}

};
};
