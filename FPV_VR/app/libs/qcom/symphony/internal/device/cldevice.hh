// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <symphony/internal/attr.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/graphics/egl.hh>
#include <symphony/internal/device/gpuopencl.hh>
#include <symphony/internal/legacy/attr.hh>
#include <symphony/internal/legacy/types.hh>
#include <symphony/range.hh>

namespace symphony {
namespace internal {

class clkernel;
class clevent;

extern std::vector<legacy::device_ptr> *s_dev_ptrs;

struct cldevice_deleter{
  static void release(cldevice *g);
};

inline
legacy::device_ptr get_default_cldevice()
{
  SYMPHONY_INTERNAL_ASSERT(s_dev_ptrs != nullptr, "cldevices not setup");
  SYMPHONY_INTERNAL_ASSERT(s_dev_ptrs->size() >= 1, "there must be at least one cldevice set up");
  return s_dev_ptrs->at(0);
}

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

class cldevice : public ref_counted_object<cldevice,
                                            symphonyptrs::default_logger,
                                            cldevice_deleter>
{

SYMPHONY_GCC_IGNORE_END("-Weffc++");

private:
  cl::Platform _ocl_platform;
  cl::Device _ocl_device;
  cl::Context _ocl_context;
  cl::CommandQueue _ocl_cmd_queue;
  cl::NDRange _ocl_global;
  cl::NDRange _ocl_local;
  cl::NDRange _ocl_offset;

public:
  cl::Context get_context() {
    return _ocl_context;
  }
  cl::CommandQueue get_cmd_queue() {
    return _ocl_cmd_queue;
  }

  void copy_to_device_blocking(const cl::Buffer buffer,
                               const void *ptr,
                               size_t size);
  void copy_from_device_blocking(const cl::Buffer buffer,
                                 void *ptr,
                                 size_t size);
  void copy_to_device_async(const cl::Buffer buffer,
                            const void *ptr, size_t size,
                            cl::Event& write_event);
  void copy_from_device_async(const cl::Buffer buffer,
                              void *ptr, size_t size,
                              cl::Event& read_event );
  void* map_async(const cl::Buffer buffer, size_t size, cl::Event& read_event);
  void unmap_async(const cl::Buffer buffer, void* mapped_ptr,
                   cl::Event& write_event);

#if SYMPHONY_HAVE_OPENCL_2_0
  void svm_map_async (void* svm_ptr, size_t size, cl::Event* read_event);
  void svm_unmap_async (void* svm_ptr, cl::Event* write_event);
#endif

private:
  cl_context_properties create_context_properties();

#ifdef SYMPHONY_HAVE_OPENCL_2_0
  inline void check_svm_info() {
    cl_device_svm_capabilities caps;
    cl_int status = get_impl().getInfo<cl_device_svm_capabilities>(CL_DEVICE_SVM_CAPABILITIES,
                                                                   &caps);
    SYMPHONY_DLOG("cldevice::check_svm_info::clGetDeviceInfo()->%d.", status);

    if (status == CL_INVALID_VALUE) {
      SYMPHONY_FATAL("You are using a CL2.0 header but SVM isn't supported on the device.");
      return;
    }
    if (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
      SYMPHONY_DLOG ("CL has Coarse-grained buffer support");
    else if (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
      SYMPHONY_DLOG ("CL has Fine-grained buffer support");
    else if (caps & (CL_DEVICE_SVM_FINE_GRAIN_BUFFER | CL_DEVICE_SVM_ATOMICS))
      SYMPHONY_DLOG ("CL has Fine-grained buffer with atomics support");
    else if (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
      SYMPHONY_DLOG ("CL has Fine-grained system support");
    else if (caps & (CL_DEVICE_SVM_FINE_GRAIN_SYSTEM | CL_DEVICE_SVM_ATOMICS))
      SYMPHONY_DLOG ("CL has Fine-grained system with atomics support");
    else
      SYMPHONY_FATAL ("CL has unknown type of SVM support");
  }
#endif
public:
  cldevice(const cl::Platform& ocl_platform,
            const cl::Device& ocl_device) : _ocl_platform(ocl_platform),
                                            _ocl_device(ocl_device),
                                            _ocl_context(),
                                            _ocl_cmd_queue(),
                                            _ocl_global(),
                                            _ocl_local(),
                                            _ocl_offset() {
#ifdef SYMPHONY_HAVE_OPENCL_2_0
    check_svm_info();
#endif

#ifdef SYMPHONY_HAVE_GLES
SYMPHONY_GCC_IGNORE_BEGIN("-Wold-style-cast")
    cl_context_properties ctx_props[] =
    {
      CL_GL_CONTEXT_KHR, (cl_context_properties)egl::get_instance().get_context(),
      CL_EGL_DISPLAY_KHR, (cl_context_properties)egl::get_instance().get_display(),
      CL_CONTEXT_PLATFORM, (cl_context_properties)(_ocl_platform)(),
      0
    };
SYMPHONY_GCC_IGNORE_END("-Wold-style-cast")
#endif

    cl_int status;
    try {
      _ocl_context = cl::Context(ocl_device,
#ifdef SYMPHONY_HAVE_GLES
                                 ctx_props,
#else
                                 nullptr,
#endif
                                 nullptr,
                                 nullptr,
                                 &status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Context()->%s", get_cl_error_string(err.err()));
    }

    try {
#ifdef SYMPHONY_HAVE_OPENCL_PROFILING
      _ocl_cmd_queue = cl::CommandQueue(_ocl_context, _ocl_device, CL_QUEUE_PROFILING_ENABLE, &status);
#else
      _ocl_cmd_queue = cl::CommandQueue(_ocl_context, _ocl_device, 0, &status);
#endif
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::CommandQueue()->%s", get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::CommandQueue()");
    }
  }

  explicit cldevice(const cl::Device& ocl_device) :
    cldevice(cl::Platform::getDefault(), ocl_device) {
#ifdef SYMPHONY_HAVE_OPENCL_2_0
    check_svm_info();
#endif
  }

  cldevice() :
    cldevice(cl::Platform::getDefault(), cl::Device::getDefault()) {
#ifdef SYMPHONY_HAVE_OPENCL_2_0
    check_svm_info();
#endif
}

  template<size_t Dims>
  void build_launch_args(const ::symphony::range<Dims>& r,
                         const ::symphony::range<Dims>& lr=::symphony::range<Dims>());

  template<size_t Dims>
  void check_launch_args(size_t work_group_size, const ::symphony::range<Dims>& r);

  template<size_t Dims>
  clevent launch_kernel(const clkernel* kernel,
                        const ::symphony::range<Dims>& r,
                        const ::symphony::range<Dims>& lr= ::symphony::range<Dims>(),
                        SYMPHONY_CL_VECTOR_CLASS<cl::Event>* events=nullptr);

  inline cl::Device get_impl() { return _ocl_device; }

  void flush() {
    cl_int status = _ocl_cmd_queue.finish();
    SYMPHONY_DLOG("cl::CommandQueue::finish()->%d",status);
    SYMPHONY_INTERNAL_ASSERT(status==CL_SUCCESS,
                         "cl::CommandQueue()::finish()->%d", status);
  }

  template<typename Attribute>
  auto get(Attribute) -> typename internal::device_attr<Attribute>::type {
    return _ocl_device.getInfo<internal::device_attr<Attribute>::cl_name>();
  }

  friend class clkernel;
  friend struct access_cldevice;
  friend struct cldevice_deleter;
};

inline
void cldevice_deleter::release(cldevice *d) {

  cl_int status = d->_ocl_cmd_queue.finish();
  SYMPHONY_DLOG("cl::CommandQueue::finish()->%d",status);
  SYMPHONY_INTERNAL_ASSERT(status==CL_SUCCESS,
                       "cl::CommandQueue::finish()->%d",
                       status);
  if(status == CL_SUCCESS)
    delete d;
}

struct access_cldevice
{
  static cl::Device& get_cl_device(legacy::device_ptr& p)
  {
    auto praw = c_ptr(p.get_raw_ptr());
    SYMPHONY_INTERNAL_ASSERT(praw != nullptr, "device_ptr is not set");
    return praw->_ocl_device;
  }

  static cl::Context& get_cl_context(legacy::device_ptr& p)
  {
    auto praw = c_ptr(p.get_raw_ptr());
    SYMPHONY_INTERNAL_ASSERT(praw != nullptr, "device_ptr is not set");
    return praw->_ocl_context;
  }

  static cl::CommandQueue& get_cl_cmd_queue(legacy::device_ptr& p)
  {
    auto praw = c_ptr(p.get_raw_ptr());
    SYMPHONY_INTERNAL_ASSERT(praw != nullptr, "device_ptr is not set");
    return praw->_ocl_cmd_queue;
  }
};

void get_devices(device_attr_device_type_gpu,
                 std::vector<legacy::device_ptr>* devices);

};

};

#endif
