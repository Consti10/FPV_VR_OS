// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/internal/util/debug.hh>

SYMPHONY_GCC_IGNORE_BEGIN("-Wshadow");
SYMPHONY_GCC_IGNORE_BEGIN("-Wdeprecated-declarations");
SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
SYMPHONY_GCC_IGNORE_BEGIN("-Wold-style-cast");
SYMPHONY_GCC_IGNORE_BEGIN("-Wunused-parameter");
SYMPHONY_GCC_IGNORE_BEGIN("-Wunused-function");
SYMPHONY_GCC_IGNORE_BEGIN("-Wmissing-braces");

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.h>

#ifdef SYMPHONY_HAVE_OPENCL_2_0

#if !(SYMPHONY_HAVE_OPENCL == 1)
#error WITH_OPENCL=1 is required to enable OpenCL 2.x features.
#endif
#if !(CL_VERSION_2_0 == 1)
#error Using a CL1.x header for CL 2.x target.
#endif

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define SYMPHONY_CL_TO_CL2
#include <CL/cl2.hpp>
#else
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 100
#define CL_HPP_CL_1_2_DEFAULT_BUILD
#include <CL/cl.hpp>
#endif

#endif

#ifdef SYMPHONY_HAVE_OPENCL_2_0
#define SYMPHONY_CL_VECTOR_CLASS cl::vector
#define SYMPHONY_CL_STRING_CLASS cl::string
#else
#include <vector>
#include <string>
#define SYMPHONY_CL_VECTOR_CLASS std::vector
#define SYMPHONY_CL_STRING_CLASS std::string
#endif

SYMPHONY_GCC_IGNORE_END("-Wmissing-braces");
SYMPHONY_GCC_IGNORE_END("-Wunused-function");
SYMPHONY_GCC_IGNORE_END("-Wunused-parameter");
SYMPHONY_GCC_IGNORE_END("-Wold-style-cast");
SYMPHONY_GCC_IGNORE_END("-Weffc++");
SYMPHONY_GCC_IGNORE_END("-Wdeprecated-declarations");
SYMPHONY_GCC_IGNORE_END("-Wshadow");
namespace symphony {

namespace internal {

const char *get_cl_error_string(cl_int error);

};
};

#endif
