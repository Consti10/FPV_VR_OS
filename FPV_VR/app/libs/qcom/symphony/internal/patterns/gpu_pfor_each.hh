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

template <typename BWAG>
void pfor_each_gpu(group* group, size_t first, size_t last, BWAG bwag) {

  if (first >= last)
    return;

  auto g_ptr = c_ptr(group);
  if (g_ptr && g_ptr->is_canceled())
    return;

  symphony::range<1> r(first, last);
  symphony::range<1> l;
  pfor_each_gpu(r, l, bwag);
}

template <size_t Dims, typename BWAG>
void pfor_each_gpu(const symphony::range<Dims>& r,
                   const symphony::range<Dims>& l, BWAG bwag)
{
  typedef typename BWAG::kernel_parameters k_params;
  typedef typename BWAG::kernel_arguments k_args;

  auto d_ptr = c_ptr((*internal::s_dev_ptrs)[0]);
  auto k_ptr = c_ptr(bwag.get_gpu_kernel());
  clkernel* kernel = k_ptr->get_cl_kernel();

  char dummy;
  void* requestor = &dummy;

  buffer_acquire_set< num_buffer_ptrs_in_tuple<k_args>::value > bas;

  gpu_kernel_dispatch::
    prepare_args<k_params, k_args>((*s_dev_ptrs)[0],
                                   k_ptr,
                                   std::forward<k_args>(
                                     bwag.get_cl_kernel_args()),
                                   bas,
                                   requestor);

  clevent event = d_ptr->launch_kernel(kernel, r, l);
  event.wait();

  bas.release_buffers(requestor);

#ifdef SYMPHONY_HAVE_OPENCL_PROFILING
    SYMPHONY_ALOG("cl::CommandQueue::enqueueNDRangeKernel() time = %0.6f us", event.get_time_ns() / 1000.0);
#endif
}

};
};

#endif
