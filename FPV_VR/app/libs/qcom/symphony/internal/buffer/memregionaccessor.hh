// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/memregion.hh>

#include <symphony/internal/buffer/memregion.hh>

namespace symphony{
namespace internal{

class memregion_base_accessor{
public:
  static internal::internal_memregion* get_internal_mr(memregion const& mr){
    return mr._int_mr;
  }
};

};
};
