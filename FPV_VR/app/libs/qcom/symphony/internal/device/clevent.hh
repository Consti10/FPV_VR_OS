// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/internal/device/gpuopencl.hh>

namespace symphony {

namespace internal {

class clevent
{
private:
  cl::Event _ocl_event;
public:
  clevent() : _ocl_event() { }
  explicit clevent(const cl::Event& event) : _ocl_event(event) { }
  inline cl::Event get_impl() {
    return _ocl_event;
  }

  inline void wait() {
     _ocl_event.wait();
  }

  double get_time_ns() {
    cl_ulong time_start, time_end;
    _ocl_event.getProfilingInfo( CL_PROFILING_COMMAND_START,&time_start);
    _ocl_event.getProfilingInfo( CL_PROFILING_COMMAND_END,&time_end);
    SYMPHONY_INTERNAL_ASSERT((time_end >= time_start),
                         "negative execution time for opencl command");
    return (time_end - time_start);
  }
};

};

};

#endif
