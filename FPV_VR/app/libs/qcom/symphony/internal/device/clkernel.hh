// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_OPENCL

#include <mutex>

#include <symphony/texture.hh>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/device/cldevice.hh>
#include <symphony/internal/device/gpuopencl.hh>

namespace symphony {

namespace internal {

extern std::vector<legacy::device_ptr> *s_dev_ptrs;

class clkernel
{
private:
  cl::Program _ocl_program;
  cl::Kernel  _ocl_kernel;
  size_t      _opt_local_size;
  std::mutex  _dispatch_mutex;
public:
  clkernel(legacy::device_ptr const& device,
            const std::string& task_str,
            const std::string& task_name,
            const std::string& build_options) :
    _ocl_program(),
    _ocl_kernel(),
    _opt_local_size(0),
    _dispatch_mutex()
  {
    auto d_ptr = internal::c_ptr(device);
    SYMPHONY_INTERNAL_ASSERT((d_ptr != nullptr), "null device ptr");

    cl_int status;
    try {
      _ocl_program = cl::Program(d_ptr->get_context(),
                                 task_str, false, &status);

      SYMPHONY_CL_VECTOR_CLASS<cl::Device> devices;
      devices.push_back(d_ptr->get_impl());
      status = _ocl_program.build(devices, build_options.c_str());
    }
    catch(cl::Error& err) {
      SYMPHONY_CL_STRING_CLASS build_log;
      _ocl_program.getBuildInfo(d_ptr->get_impl(),
                                CL_PROGRAM_BUILD_LOG, &build_log);
      SYMPHONY_FATAL("cl::Program::build()->%s\n build_log: %s",
                  get_cl_error_string(err.err()), build_log.c_str());
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::Program::build()");
    }

    try {
      _ocl_kernel = cl::Kernel(_ocl_program, task_name.c_str(), &status);
      _opt_local_size = _ocl_kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(d_ptr->get_impl(), &status);
    }
    catch(cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel()->%s", get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::Kernel()");
    }

  }

  clkernel(legacy::device_ptr const& device,
            void const* kernel_bin,
            size_t kernel_size,
            const std::string& kernel_name,
            const std::string& build_options) :
    _ocl_program(),
    _ocl_kernel(),
    _opt_local_size(0),
    _dispatch_mutex()
  {
    auto d_ptr = internal::c_ptr(device);
    SYMPHONY_INTERNAL_ASSERT((d_ptr != nullptr), "null device ptr");
    SYMPHONY_CL_VECTOR_CLASS<cl::Device> devices;
    cl_int status;

    try {
      devices.push_back(d_ptr->get_impl());
      SYMPHONY_CL_VECTOR_CLASS<std::pair<void const*, size_t>> binaries;
      binaries.push_back(std::make_pair(kernel_bin, kernel_size));
      _ocl_program = cl::Program(d_ptr->get_context(),
                                 devices,
                                 binaries,
                                 nullptr,
                                 &status);
    }
    catch(cl::Error &err) {
      SYMPHONY_FATAL("cl::Program(binary %p, length %zu)->%s",
                 kernel_bin,
                 kernel_size,
                 get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in creating a program");
    }

    try {
      status = _ocl_program.build(devices, build_options.c_str());
    }
    catch(cl::Error& err) {
      SYMPHONY_CL_STRING_CLASS build_log;
      _ocl_program.getBuildInfo(d_ptr->get_impl(),
                                CL_PROGRAM_BUILD_LOG,
                                &build_log);
      SYMPHONY_FATAL("cl::Program::build()->%s\n build_log: %s",
                  get_cl_error_string(err.err()),
                  build_log.c_str());
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::Program::build()");
    }

    try {
      _ocl_kernel = cl::Kernel(_ocl_program, kernel_name.c_str(), &status);
      _opt_local_size = _ocl_kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(d_ptr->get_impl(), &status);
    }
    catch(cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel()->%s", get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::Kernel()");
    }

  }

  inline size_t get_optimal_local_size() const {
    return _opt_local_size;
  }

  inline const cl::Kernel get_impl() const {
    return _ocl_kernel;
  }

  std::pair<void const*, size_t> get_cl_kernel_binary() const
  {
    void*  bin_ptr;
    size_t bin_size;

    try {
      cl_uint ndev;
      _ocl_program.getInfo(CL_PROGRAM_NUM_DEVICES, &ndev);
      if(ndev == 0)
        SYMPHONY_FATAL("No OpenCL devices found");

      auto sizes = new size_t[ndev];
      _ocl_program.getInfo(CL_PROGRAM_BINARY_SIZES, sizes);
      bin_size = sizes[0];

      auto binaries = new unsigned char*[ndev];

      binaries[0] = new unsigned char[bin_size];
      for (size_t i = 1; i < ndev; i++)
        binaries[i] = nullptr;
      _ocl_program.getInfo(CL_PROGRAM_BINARIES, binaries);

      bin_ptr = binaries[0];

      delete[] sizes;
      delete[] binaries;
    }
    catch(cl::Error& err) {
      SYMPHONY_FATAL("cl::Program::getInfo()->%s", get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::Program::getInfo()");
    }

    return std::make_pair(bin_ptr, bin_size);
  }

  inline std::mutex& access_dispatch_mutex() {
    return _dispatch_mutex;
  }

  inline void set_arg(size_t arg_index,
                      symphony::graphics::internal::base_texture_cl* tex_cl)
  {
    SYMPHONY_INTERNAL_ASSERT(tex_cl != nullptr, "null texture_cl");

    try {

      cl_int status = _ocl_kernel.setArg(arg_index, tex_cl->_img);
      SYMPHONY_UNUSED(status);
      SYMPHONY_DLOG("cl::Kernel::setArg(%zu, texture %p)->%d",
                arg_index, tex_cl, status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setArg(%zu, texture %p)->%s",
                 arg_index, tex_cl, get_cl_error_string(err.err()));
    }
  }

  template<symphony::graphics::addressing_mode addrMode, symphony::graphics::filter_mode filterMode>
  inline void set_arg(size_t                                                   arg_index,
                      symphony::graphics::sampler_ptr<addrMode, filterMode> const& sampler_ptr)
  {
    auto const& psampler = c_ptr(sampler_ptr);
    SYMPHONY_INTERNAL_ASSERT((psampler != nullptr), "null sampler_cl");

    try {

      cl_int status = clSetKernelArg(_ocl_kernel(),
                                     arg_index,
                                     sizeof(cl_sampler),
                                     static_cast<void*>(&(psampler->_sampler)));
      if(status != CL_SUCCESS)
        SYMPHONY_FATAL("clSetKernelArg(%zu, sampler %p)->%d",
                   arg_index, psampler, status);

      SYMPHONY_DLOG("cl::Kernel::setArg(%zu, sampler %p)->%d",
                arg_index, psampler, status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setArg(%zu, sampler %p)->%s",
                 arg_index, psampler, get_cl_error_string(err.err()));
    }
  }

  inline void set_arg(size_t arg_index, cl::Buffer const& cl_buffer) {
    try {

      cl_int status = _ocl_kernel.setArg(arg_index, cl_buffer);
      SYMPHONY_UNUSED(status);
      SYMPHONY_DLOG("cl::Kernel::setArg(%zu, %p)->%d", arg_index, &cl_buffer, status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setArg(%zu, %p)->%s",
                 arg_index, &cl_buffer, get_cl_error_string(err.err()));
    }
  }

#if SYMPHONY_HAVE_OPENCL_2_0

  inline void set_arg_svm(size_t arg_index, void* pointer) {
    try {

      cl_int status = _ocl_kernel.setArg(arg_index, pointer);
      SYMPHONY_UNUSED(status);
      SYMPHONY_DLOG("cl::Kernel::setSVMArg(%zu, %p)->%d", arg_index, pointer, status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setSVMArg(%zu, %p)->%s",
                 arg_index, pointer, get_cl_error_string(err.err()));
    }
  }
#endif

  template<typename T>
  void set_arg(size_t arg_index, T value) {
    try {
      cl_int status = _ocl_kernel.setArg(arg_index, sizeof(T), &value);
      SYMPHONY_UNUSED(status);
      SYMPHONY_DLOG("cl::Kernel::setArg(%zu, %zu, %p)->%d",
                arg_index,
                sizeof(T),
                &value,
                status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setArg(%zu, %zu, %p)->%s",
                 arg_index, sizeof(T), &value, get_cl_error_string(err.err()));

    }
  }

  inline void set_arg_local_alloc(size_t arg_index, size_t num_bytes_to_local_alloc) {
    try {

      cl_int status = _ocl_kernel.setArg(arg_index, num_bytes_to_local_alloc, nullptr);
      SYMPHONY_UNUSED(status);
      SYMPHONY_DLOG("cl::Kernel::setArg(%zu, %zu bytes)->%d", arg_index, num_bytes_to_local_alloc, status);
    }
    catch (cl::Error& err) {
      SYMPHONY_FATAL("cl::Kernel::setArg(%zu, %zu bytes)->%s",
                 arg_index, num_bytes_to_local_alloc, get_cl_error_string(err.err()));
    }
  }

  template<size_t index = 0, typename... Kargs>
  typename std::enable_if<(index == sizeof...(Kargs)), void>::type
   set_args(const std::tuple<Kargs...>&) {

  }

  template<size_t index =0, typename... Kargs>
  typename std::enable_if<(index < sizeof...(Kargs)), void>::type
   set_args(const std::tuple<Kargs...>& t) {
    set_arg(index, std::get<index>(t));
    set_args<index+1, Kargs...>(t);
  }
};

};
};

#endif
