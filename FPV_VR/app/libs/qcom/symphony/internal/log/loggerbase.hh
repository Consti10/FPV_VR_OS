// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/log/events.hh>
#include <symphony/internal/log/objectid.hh>

#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {
namespace log {

class logger_base {
public:
  virtual ~logger_base(){}

  enum class logger_id : int8_t {
    unknown = -1,
    corelogger = 0,
    schedulerlogger = 1,
    userlogger = 2
   };
};

};
};
};

