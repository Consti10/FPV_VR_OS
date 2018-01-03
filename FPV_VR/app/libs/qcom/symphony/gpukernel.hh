// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/task/gpukernel.hh>
#include <symphony/kernel.hh>

namespace symphony {

template<typename T>
class local;

namespace beta {

class cl_t {};

class gl_t {};

cl_t const cl {};

gl_t const gl {};

};

#ifdef SYMPHONY_HAVE_GPU
SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename ...Args>
class gpu_kernel : private internal::gpu_kernel_implementation<Args...>
{
  using parent = internal::gpu_kernel_implementation<Args...>;

  enum { num_kernel_args = sizeof...(Args) };

  enum kernel_type { CL, GL };
public:

#ifdef SYMPHONY_HAVE_OPENCL

  gpu_kernel(std::string const& cl_kernel_str,
             std::string const& cl_kernel_name,
             std::string const& cl_build_options = "") :
    parent(cl_kernel_str, cl_kernel_name, cl_build_options),
    _kernel_type(CL)
  {}

  gpu_kernel(beta::cl_t const&,
             std::string const& cl_kernel_str,
             std::string const& cl_kernel_name,
             std::string const& cl_build_options = "") :
    parent(cl_kernel_str, cl_kernel_name, cl_build_options),
    _kernel_type(CL)
  {}
#endif

#ifdef SYMPHONY_HAVE_GLES

  gpu_kernel(beta::gl_t const&,
             std::string const& gl_kernel_str) :
    parent(gl_kernel_str),
    _kernel_type(GL)
  {}
#endif

#ifdef SYMPHONY_HAVE_OPENCL

  gpu_kernel(void const*        cl_kernel_bin,
             size_t             cl_kernel_len,
             std::string const& cl_kernel_name,
             std::string const& cl_build_options = "") :
    parent(cl_kernel_bin, cl_kernel_len, cl_kernel_name, cl_build_options),
    _kernel_type(CL)
  {}

  gpu_kernel(beta::cl_t const&,
             void const*        cl_kernel_bin,
             size_t             cl_kernel_len,
             std::string const& cl_kernel_name,
             std::string const& cl_build_options = "") :
    parent(cl_kernel_bin, cl_kernel_len, cl_kernel_name, cl_build_options),
    _kernel_type(CL)
  {}

  std::pair<void const*, size_t> get_cl_kernel_binary() const
  {
    SYMPHONY_API_ASSERT(is_cl(), "CL kernel binary info requested for a non-OpenCL kernel");
    return parent::get_cl_kernel_binary();
  }
#endif

  SYMPHONY_DEFAULT_METHOD(gpu_kernel(gpu_kernel const&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel& operator=(gpu_kernel const&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel(gpu_kernel&&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel& operator=(gpu_kernel&&));

  ~gpu_kernel() {}

  bool is_cl() const { return _kernel_type == CL; }

  bool is_gl() const { return _kernel_type == GL; }

private:

  kernel_type const _kernel_type;

  template<typename Code, class Enable>
  friend
  struct symphony::internal::task_factory_dispatch;

  template<typename GPUKernel, typename ...CallArgs>
  friend
  void symphony::internal::execute_gpu(GPUKernel const& gk, CallArgs&&... args);

  template<typename GPUKernel, size_t Dims, typename ...CallArgs>
  friend
  struct symphony::internal::executor;
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");
#endif

#ifdef SYMPHONY_HAVE_OPENCL

template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(std::string const& cl_kernel_str,
                                      std::string const& cl_kernel_name,
                                      std::string const& cl_build_options = "")
{
  return gpu_kernel<Args...>(cl_kernel_str, cl_kernel_name, cl_build_options);
}
#endif

namespace beta {

#ifdef SYMPHONY_HAVE_OPENCL

template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(beta::cl_t const&,
                                      std::string const& cl_kernel_str,
                                      std::string const& cl_kernel_name,
                                      std::string const& cl_build_options = "")
{
  return gpu_kernel<Args...>(beta::cl, cl_kernel_str, cl_kernel_name, cl_build_options);
}
#endif

#ifdef SYMPHONY_HAVE_GLES

template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(beta::gl_t const&,
                                      std::string const& gl_kernel_str)
{
  return gpu_kernel<Args...>(beta::gl, gl_kernel_str);
}
#endif

};

#ifdef SYMPHONY_HAVE_OPENCL

template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(void const*        cl_kernel_bin,
                                      size_t             cl_kernel_len,
                                      std::string const& cl_kernel_name,
                                      std::string const& cl_build_options = "")
{
  return gpu_kernel<Args...>(cl_kernel_bin,
                             cl_kernel_len,
                             cl_kernel_name,
                             cl_build_options);
}
#endif

namespace beta {

#ifdef SYMPHONY_HAVE_OPENCL

template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(beta::cl_t const&,
                                      void const*        cl_kernel_bin,
                                      size_t             cl_kernel_len,
                                      std::string const& cl_kernel_name,
                                      std::string const& cl_build_options = "")
{
  return gpu_kernel<Args...>(beta::cl,
                             cl_kernel_bin,
                             cl_kernel_len,
                             cl_kernel_name,
                             cl_build_options);
}
#endif

#ifdef SYMPHONY_HAVE_GLES
template<typename ...Args>
gpu_kernel<Args...> create_gpu_kernel(beta::gl_t const&,
                                      void const* gl_kernel_bin,
                                      size_t      gl_kernel_len)
{
  SYMPHONY_UNIMPLEMENTED("Symphony does not support creation of precompiled OpenGL ES kernels.");
  return gpu_kernel<Args...>(beta::gl, gl_kernel_bin, gl_kernel_len);
}
#endif

#ifdef SYMPHONY_HAVE_GPU

template <size_t Dim, typename ...Args>
struct call_tuple<Dim, gpu_kernel<Args...>> {
  using type = std::tuple<symphony::range<Dim>,
                          typename internal::strip_buffer_dir<Args>::type...>;
};
#endif

};

};
