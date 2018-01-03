// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if __hexagon__
#include <symphony/internal/pointkernel/macrodefinitions_hexagon_1d.h>
#include <symphony/internal/pointkernel/macrodefinitions_hexagon_2d.h>
#else
#include <symphony/internal/pointkernel/converttype.h>
#include <symphony/internal/pointkernel/macrodefinitions_1d.hh>
#include <symphony/internal/pointkernel/macrodefinitions_2d.hh>
#include <symphony/internal/pointkernel/vectype.h>
#endif
