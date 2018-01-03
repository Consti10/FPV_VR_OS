// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/exceptions.hh>

namespace symphony {

namespace internal {

class api_assert_exception : public error_exception
{
public:
  api_assert_exception(std::string msg,
                   const char* filename, int lineno, const char* funcname)
    : error_exception(msg, filename, lineno, funcname)
  { }
};

class assert_exception : public error_exception
{
public:
  assert_exception(std::string msg,
                   const char* filename, int lineno, const char* funcname)
    : error_exception(msg, filename, lineno, funcname)
  { }
};

class fatal_exception : public error_exception
{
public:
  fatal_exception(std::string msg,
                   const char* filename, int lineno, const char* funcname)
    : error_exception(msg, filename, lineno, funcname)
  { }
};

};

};
