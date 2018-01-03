// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/internal/device/gpuopencl.hh>

namespace symphony {
namespace graphics {
namespace internal {
void setup_supported_image_format_set();
void delete_supported_image_format_set();

void setup_image_format_conversions();
void cleanup_image_format_conversions();
};
};
};

namespace symphony {
namespace graphics {
namespace internal {
struct image_format_less
{
  bool operator () (cl::ImageFormat const& img_format1, cl::ImageFormat const& img_format2) const
  {
    return (img_format1.image_channel_order < img_format2.image_channel_order ||
           ((img_format1.image_channel_order == img_format2.image_channel_order) &&
           (img_format1.image_channel_data_type < img_format2.image_channel_data_type)));
  }
};
};
};
};
#endif
