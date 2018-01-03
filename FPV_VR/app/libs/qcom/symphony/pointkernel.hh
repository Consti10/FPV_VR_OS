// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include "internal/pointkernel/macrodefinitions.hh"

#if !__hexagon__

#include <symphony/internal/pointkernel/pointkernel.hh>

namespace symphony{

namespace beta{

template <typename T>
typename std::tuple_element<0,T>::type create_point_kernel(std::string global_string = "")
{
  SYMPHONY_UNUSED(global_string);
}
};

};

#endif
