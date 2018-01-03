// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstdint>
#include <iostream>
#include <typeinfo>
#include <string>
#include <type_traits>

#include <symphony/internal/device/gpuopencl.hh>

namespace symphony{

namespace internal {

struct task_attr_values {
  enum {
    NONE =              0x000,
    LONG_RUNNING =      0x001,
    BLOCKING =          LONG_RUNNING << 1,
    ANONYMOUS =         BLOCKING << 1,
    STUB =              ANONYMOUS << 1,
    NOT_CANCELABLE =    STUB << 1,
    PFOR =              NOT_CANCELABLE << 1,
    YIELD =             PFOR << 1,
    OPENCL =            YIELD << 1,
    TRIGGER =           OPENCL << 1,
    INLINED =           TRIGGER << 1,
    HEXAGON =           INLINED << 1,
    CPU     =           HEXAGON << 1,
    API20   =           CPU << 1,
    BIG     =           API20 << 1,
    LITTLE  =           BIG << 1
  };
};

struct task_attr_base {};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

struct task_attr_none : public task_attr_base {
  enum {value = task_attr_values::NONE};
  enum {conflicts_with = task_attr_values::NONE};

  task_attr_none(){}
};

struct task_attr_long_running : public task_attr_base {
  enum {value = task_attr_values::LONG_RUNNING};
  enum {conflicts_with = task_attr_values::STUB |
                         task_attr_values::PFOR
  };

  task_attr_long_running(){}
};

struct task_attr_blocking : public task_attr_base {
  enum {value = task_attr_values::BLOCKING};
  enum {conflicts_with = task_attr_values::STUB |
                         task_attr_values::PFOR
       };

  task_attr_blocking(){}
};

struct task_attr_big : public task_attr_base {
  enum {value = task_attr_values::BIG};
  enum {conflicts_with = task_attr_values::STUB |
                         task_attr_values::PFOR |
                         task_attr_values::BLOCKING |
                         task_attr_values::LITTLE |
                         task_attr_values::YIELD
       };

  task_attr_big(){}
};

struct task_attr_little : public task_attr_base {
  enum {value = task_attr_values::LITTLE};
  enum {conflicts_with = task_attr_values::STUB |
                         task_attr_values::PFOR |
                         task_attr_values::BLOCKING |
                         task_attr_values::BIG |
                         task_attr_values::YIELD
       };

  task_attr_little(){}
};

struct task_attr_anonymous : public task_attr_base {
  enum {value = task_attr_values::ANONYMOUS};
  enum {conflicts_with = task_attr_values::STUB};

  task_attr_anonymous(){}
};

struct task_attr_stub : public task_attr_base {
  enum {
    value = task_attr_values::STUB
  };
  enum {
    conflicts_with = task_attr_values::BLOCKING |
                     task_attr_values::LONG_RUNNING |
                     task_attr_values::ANONYMOUS |
                     task_attr_values::PFOR |
                     task_attr_values::TRIGGER
  };

  task_attr_stub(){}
};

struct task_attr_pfor : public task_attr_base {
  enum {
    value = task_attr_values::PFOR
  };

  enum {
    conflicts_with = task_attr_values::BLOCKING |
                     task_attr_values::LONG_RUNNING |
                     task_attr_values::STUB
  };

  task_attr_pfor(){}
};

struct task_attr_non_cancelable : public task_attr_base {
  enum {value = task_attr_values::NOT_CANCELABLE};
  enum {conflicts_with = task_attr_values::NONE};

  task_attr_non_cancelable(){}
};

struct task_attr_yield : public task_attr_base {
  enum {
    value = task_attr_values::YIELD
  };
  enum {
    conflicts_with = task_attr_values::BLOCKING |
                     task_attr_values::LONG_RUNNING |
                     task_attr_values::ANONYMOUS |
                     task_attr_values::TRIGGER
  };

  task_attr_yield(){}
};

struct task_attr_opencl : public task_attr_base {
  enum { value = task_attr_values::OPENCL };
  enum { conflicts_with = task_attr_values::NONE };

  task_attr_opencl() {}
};

struct task_attr_hexagon : public task_attr_base {
  enum { value = task_attr_values::HEXAGON };
  enum { conflicts_with = task_attr_values::NONE };

  task_attr_hexagon() {}
};

struct task_attr_cpu : public task_attr_base {
  enum { value = task_attr_values::CPU };
  enum { conflicts_with = task_attr_values::OPENCL |
                          task_attr_values::HEXAGON };

  task_attr_cpu() {}
};

struct task_attr_api20 : public task_attr_base {
  enum { value = task_attr_values::API20 };
  enum { conflicts_with = task_attr_values::NONE };

  task_attr_api20() {}
};

struct task_attr_trigger : public task_attr_base {
  enum {
    value = task_attr_values::TRIGGER
  };
  enum {
    conflicts_with = task_attr_values::BLOCKING |
                     task_attr_values::LONG_RUNNING |
                     task_attr_values::ANONYMOUS |
                     task_attr_values::STUB |
                     task_attr_values::YIELD
  };

  task_attr_trigger(){}
};

struct task_attr_inlined : public task_attr_base {
  enum {
    value = task_attr_values::INLINED
  };
  enum {
    conflicts_with = task_attr_values::OPENCL |
                     task_attr_values::STUB |
                     task_attr_values::TRIGGER |
                     task_attr_values::YIELD
  };

  task_attr_inlined(){}
};

#ifdef SYMPHONY_HAVE_GPU
struct device_attr_available {
  device_attr_available(){}
};

struct device_attr_compiler_available {
  device_attr_compiler_available(){}
};

struct device_attr_double_fp_config {
  device_attr_double_fp_config(){}
};

struct device_attr_endian_little {
  device_attr_endian_little(){}
};

struct device_attr_ecc_support {
  device_attr_ecc_support(){}
};

struct device_attr_global_memsize {
  device_attr_global_memsize(){}
};

struct device_attr_local_memsize {
  device_attr_local_memsize(){}
};

struct device_attr_max_clock_freq {
  device_attr_max_clock_freq(){}
};

struct device_attr_max_compute_units {
  device_attr_max_compute_units(){}
};

struct device_attr_max_workgroup_size {
  device_attr_max_workgroup_size(){}
};

struct device_attr_max_work_item_dims {
  device_attr_max_work_item_dims(){}
};

struct device_attr_max_work_item_sizes {
  device_attr_max_work_item_sizes(){}
};

struct device_attr_device_name {
  device_attr_device_name(){}
};

struct device_attr_device_vendor {
  device_attr_device_vendor(){}
};

struct device_attr_device_vendor_id {
  device_attr_device_vendor_id(){}
};

struct device_attr_device_type_cpu {
  device_attr_device_type_cpu(){}
};

struct device_attr_device_type_gpu {
  device_attr_device_type_gpu(){}
};

struct device_attr_device_type_hexagon {
  device_attr_device_type_hexagon(){}
};

struct device_attr_device_type_default {
  device_attr_device_type_default(){}
};

struct buffer_attr_read_only {
  buffer_attr_read_only(){}
};

struct buffer_attr_write_only {
  buffer_attr_write_only(){}
};

struct buffer_attr_read_write {
  buffer_attr_read_write(){}
};

#ifdef SYMPHONY_HAVE_OPENCL

template<cl_device_info name>
struct cl_attr { enum {cl_name = name}; };

template<typename T>
struct Type { typedef T type; };

template<typename T>
struct device_attr { };

template<typename T>
struct buffer_attr { };

template<>
struct device_attr<device_attr_available> : public Type<bool>,
                                   cl_attr<CL_DEVICE_AVAILABLE> { };

template<>
struct device_attr<device_attr_compiler_available> : public Type<bool>,
                                   cl_attr<CL_DEVICE_COMPILER_AVAILABLE> { };

template<>
struct device_attr<device_attr_double_fp_config> : public Type<int>,
                                   cl_attr<CL_DEVICE_DOUBLE_FP_CONFIG> { };
template<>
struct device_attr<device_attr_endian_little> : public Type<bool>,
                                   cl_attr<CL_DEVICE_ENDIAN_LITTLE> { };
template<>
struct device_attr<device_attr_ecc_support> : public Type<bool>,
                                   cl_attr<CL_DEVICE_ERROR_CORRECTION_SUPPORT>
                                   { };
template<>
struct device_attr<device_attr_global_memsize> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_GLOBAL_MEM_SIZE> { };
template<>
struct device_attr<device_attr_local_memsize> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_LOCAL_MEM_SIZE> { };
template<>
struct device_attr<device_attr_max_clock_freq> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_MAX_CLOCK_FREQUENCY> { };
template<>
struct device_attr<device_attr_max_compute_units> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_MAX_COMPUTE_UNITS> { };
template<>
struct device_attr<device_attr_max_workgroup_size> : public Type<size_t>,
                                   cl_attr<CL_DEVICE_MAX_WORK_GROUP_SIZE> { };
template<>
struct device_attr<device_attr_max_work_item_dims> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>
                                   { };
template<>
struct device_attr<device_attr_max_work_item_sizes> :
                                   public Type<std::vector<size_t> >,
                                   cl_attr<CL_DEVICE_MAX_WORK_ITEM_SIZES> { };
template<>
struct device_attr<device_attr_device_name> : public Type<std::string>,
                                   cl_attr<CL_DEVICE_NAME> { };
template<>
struct device_attr<device_attr_device_vendor> : public Type<std::string>,
                                   cl_attr<CL_DEVICE_VENDOR> { };
template<>
struct device_attr<device_attr_device_vendor_id> : public Type<uint32_t>,
                                   cl_attr<CL_DEVICE_VENDOR_ID> { };

template<>
struct device_attr<device_attr_device_type_cpu> : public Type<int>,
                                   cl_attr<CL_DEVICE_TYPE_CPU> { };

template<>
struct device_attr<device_attr_device_type_gpu> : public Type<int>,
                                   cl_attr<CL_DEVICE_TYPE_GPU> { };

template<>
struct device_attr<device_attr_device_type_default> : public Type<int>,
                                   cl_attr<CL_DEVICE_TYPE_DEFAULT> { };

template<>
struct buffer_attr<buffer_attr_read_only> : public Type<int>,
                                   cl_attr<CL_MEM_READ_ONLY> { };

template<>
struct buffer_attr<buffer_attr_write_only> : public Type<int>,
                                   cl_attr<CL_MEM_WRITE_ONLY> { };

template<>
struct buffer_attr<buffer_attr_read_write> : public Type<int>,
                                   cl_attr<CL_MEM_READ_WRITE> { };

#endif

#endif

SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<int FIRST, int SECOND, typename... TS>
struct find_conflict;

template<int FIRST, int SECOND, typename A>
struct find_conflict<FIRST, SECOND, A>{
  enum {value = false };
  enum {first = -1};
  enum {second = -1};
};

template<int FIRST, int SECOND, typename A, typename B>
struct find_conflict<FIRST, SECOND, A,B>{
  enum {value = (A::conflicts_with & B::value)  == 0? false : true};
  enum {first = value? FIRST : -1};
  enum {second = value? SECOND : -1};
};

template<int FIRST, int SECOND, typename A1, typename A2, typename ...AS>
struct find_conflict<FIRST, SECOND,A1, A2, AS...>{

  enum {
    value = (find_conflict<FIRST, SECOND, A1,A2>::value) ?
    true :
    static_cast<bool>(find_conflict<FIRST, SECOND+1,A1,AS...>::value)
  };
  enum {
    first = (find_conflict<FIRST, SECOND, A1,A2>::value) ?
    FIRST : find_conflict<FIRST, SECOND+1, A1, AS...>::first
  };
  enum {
    second = (find_conflict<FIRST, SECOND, A1,A2>::value) ?
    SECOND : find_conflict<FIRST, SECOND+1, A1, AS...>::second
  };
};

template<int POS, typename... TS>
struct traverse_attrs_from;

template<int POS, typename A>
struct traverse_attrs_from<POS, A>{
  enum {value = false};
  enum {first = -1};
  enum {second = -1};
};

template<int POS, typename A, typename B, typename ...Z>
struct traverse_attrs_from<POS, A,B, Z...>{
  typedef find_conflict<POS, POS+1, A, B, Z...> current;
  typedef find_conflict<POS+1, POS+2, B, Z...> next;
  enum {
    value = current::value ? true : static_cast<bool>(next::value)
  };
  enum {
    first = current::value ?
    static_cast<int>(current::first) : static_cast<int>(next::first)
  };
  enum {
    second = current::value ?
    static_cast<int>(current::second) : static_cast<int>(next::second)};
};

template<typename A, typename... AS>
struct find_task_attr_conflict {
  enum {value = traverse_attrs_from<0, A, AS...>::value};
  enum {first = traverse_attrs_from<0, A, AS...>::first};
  enum {second = traverse_attrs_from<0, A, AS...>::second};
};

template<typename... TS>
struct build_attr_mask;

template<>
struct build_attr_mask<>{
  enum {value = task_attr_values::NONE};
};

template<typename A1, typename ...AS>
struct build_attr_mask<A1, AS...>{
  enum {value = A1::value | build_attr_mask<AS...>::value};
};

template<typename A, typename...AS>
bool test_attr_conflict_loc(int& loc1, int& loc2, A, AS...) {
  loc1 = internal::find_task_attr_conflict<A, AS...>::first;
  loc2 = internal::find_task_attr_conflict<A, AS...>::second;
  return internal::find_task_attr_conflict<A, AS...>::value;
}

template<typename A, typename...AS>
bool test_attr_conflict(A, AS...) {
  return internal::find_task_attr_conflict<A, AS...>::value;
}

namespace attr {
static const internal::task_attr_anonymous anonymous;
static const internal::task_attr_long_running long_running;
static const internal::task_attr_none none;
static const internal::task_attr_stub stub;
static const internal::task_attr_pfor pfor;
static const internal::task_attr_non_cancelable non_cancelable;
static const internal::task_attr_yield yield;
static const internal::task_attr_trigger trigger;
static const internal::task_attr_inlined inlined;
static const internal::task_attr_api20 api20;
};

};

};
