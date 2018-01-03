// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/legacy/attrobjs.hh>

#include <symphony/internal/attr.hh>

namespace symphony{
namespace internal{
namespace legacy{

#ifdef SYMPHONY_HAVE_GPU
namespace device_attr {

static const internal::device_attr_available is_available;
static const internal::device_attr_compiler_available compiler_available;
static const internal::device_attr_double_fp_config double_fp_config;
static const internal::device_attr_endian_little little_endian;
static const internal::device_attr_ecc_support ecc_support;
static const internal::device_attr_global_memsize global_mem_size;
static const internal::device_attr_local_memsize local_mem_size;
static const internal::device_attr_max_clock_freq max_clock_freq;
static const internal::device_attr_max_compute_units max_compute_units;
static const internal::device_attr_max_workgroup_size max_work_group_size;
static const internal::device_attr_max_work_item_dims max_work_item_dims;
static const internal::device_attr_max_work_item_sizes max_work_item_sizes;
static const internal::device_attr_device_name name;
static const internal::device_attr_device_vendor vendor;
static const internal::device_attr_device_type_gpu gpu;

};

namespace buffer_attr {

static const internal::buffer_attr_read_only read_only;
static const internal::buffer_attr_write_only write_only;
static const internal::buffer_attr_read_write read_write;

};
#endif

namespace attr {

static const internal::task_attr_blocking blocking;

static const internal::task_attr_yield yield;

static const internal::task_attr_cpu cpu;

static const internal::task_attr_opencl gpu;

static const internal::task_attr_hexagon hexagon;

static const internal::task_attr_big big;

static const internal::task_attr_little little;

};

class task_attrs;

SYMPHONY_CONSTEXPR task_attrs create_task_attrs();

template<typename Attribute, typename ...Attributes>
constexpr task_attrs create_task_attrs(Attribute const& attr1,
                                       Attributes const&...attrn);

template<typename Attribute>
constexpr bool has_attr(task_attrs const& attrs, Attribute const& attr);

template<typename Attribute>
constexpr task_attrs remove_attr(task_attrs const& attrs,
                                 Attribute const& attr);

template<typename Attribute>
const task_attrs add_attr(task_attrs const& attrs, Attribute const& attr);

template<typename Attribute>
constexpr bool has_attr(task_attrs const& attrs, Attribute const&) {
  static_assert(std::is_base_of<internal::task_attr_base, Attribute>::value,
                "Invalid task attribute.");
  return ((attrs._mask & Attribute::value) != 0);
}

template<typename Attribute>
constexpr task_attrs
remove_attr(task_attrs const& attrs, Attribute const&)
{
  static_assert(std::is_base_of<internal::task_attr_base, Attribute>::value,
                "Invalid task attribute.");
  return task_attrs(attrs._mask & ~Attribute::value);
}

template<typename Attribute>
const task_attrs add_attr(task_attrs const& attrs, Attribute const&) {
  static_assert(std::is_base_of<internal::task_attr_base, Attribute>::value,
                "Invalid task attribute.");
  SYMPHONY_API_ASSERT((Attribute::conflicts_with & attrs._mask) == 0,
                  "Conflicting task attributes");
  return task_attrs(attrs._mask | Attribute::value);
}

inline SYMPHONY_CONSTEXPR task_attrs create_task_attrs() {
  return task_attrs(0);
}

#ifdef ONLY_FOR_DOXYGEN
#error The compiler should not see these methods

template<typename Attribute, typename ...Attributes>
inline constexpr task_attrs create_task_attrs(Attribute const& attr1,
                                              Attributes const&...attrn);

#else

template<typename Attribute, typename ...Attributes>
inline constexpr task_attrs create_task_attrs(Attribute const&,
                                              Attributes const&...) {
  static_assert(internal::
                find_task_attr_conflict<Attribute,
                Attributes...>::value == false,
                "Conflicting task attributes");
  return task_attrs(internal::build_attr_mask<Attribute,
                                              Attributes...>::value);
}

#endif

};
};
};
