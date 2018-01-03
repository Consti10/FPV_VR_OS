// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/range.hh>

namespace symphony {

namespace internal{

template <size_t DIMS, typename BWAG>
void gpu_custom_launch_local_size(const symphony::range<DIMS>& r, const symphony::range<DIMS>& lr, BWAG bwag)
{
  typedef typename BWAG::kernel_parameters k_params;
  typedef typename BWAG::kernel_arguments k_args;

  auto d_ptr = c_ptr(internal::s_dev_ptrs->at(0));
  auto k_ptr = c_ptr(bwag.get_gpu_kernel());
  clkernel* kernel = k_ptr->get_cl_kernel();

  char dummy;
  void* requestor = &dummy;

  gpu_kernel_dispatch::
    prepare_args<k_params, k_args>(s_dev_ptrs->at(0),
                                   kernel,
                                   std::forward<k_args>(
                                   bwag.get_cl_kernel_args()),
                                   requestor);

  clevent event = d_ptr->launch_kernel(kernel, r, lr);
  event.wait();

  gpu_kernel_dispatch::
    release_any_api20_buffers<k_args>(std::forward<k_args>(bwag.get_cl_kernel_args()),
                                      requestor);
  #ifdef SYMPHONY_HAVE_OPENCL_PROFILING
    SYMPHONY_ALOG("cl::CommandQueue::enqueueNDRangeKernel() time = %0.6f us",
              (event.get_time_ns() / 1000.0));
  #endif
}

};
};

#endif
