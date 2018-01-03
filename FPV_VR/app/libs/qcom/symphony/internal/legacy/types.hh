// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/util/symphonyptrs.hh>

namespace symphony {
namespace internal {

class group;
class hexagonbuffer;

#ifdef SYMPHONY_HAVE_OPENCL
class cldevice;
#endif

namespace legacy {

#ifdef SYMPHONY_HAVE_GPU

template<typename T>
class buffer;

template<typename...Kargs>
class gpukernel;

#ifdef SYMPHONY_HAVE_OPENCL

typedef internal::symphony_shared_ptr<internal::cldevice> device_ptr;
#else

typedef struct dummy_device_ptr {} device_ptr;
#endif

template<typename... Kargs>
using kernel_ptr = internal::symphony_shared_ptr< ::symphony::internal::legacy::gpukernel<Kargs...> >;

#endif

#ifdef SYMPHONY_HAVE_QTI_HEXAGON
typedef internal::symphony_shared_ptr<internal::hexagonbuffer> hexagonbuffer_ptr;
#endif

};
};
};
