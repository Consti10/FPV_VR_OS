// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_GPU

#include <symphony/range.hh>

#include <symphony/internal/legacy/gpukernel.hh>
#include <symphony/internal/legacy/group.hh>
#include <symphony/internal/legacy/task.hh>

namespace symphony {
namespace internal {

template<typename GPUKernel, typename ...CallArgs>
void execute_gpu(GPUKernel const& gk, CallArgs&&... args);

template<typename GPUKernel, size_t Dims, typename ...CallArgs>
struct executor;

template<typename BAS, typename Executor>
struct process_executor_buffers;

};
};

namespace symphony {
namespace internal {

inline
symphony::internal::legacy::device_ptr get_gpu_device() {
#ifdef SYMPHONY_HAVE_OPENCL

  SYMPHONY_INTERNAL_ASSERT(!::symphony::internal::s_dev_ptrs->empty(),
                       "No OpenCL GPU devices found on the platform");
  ::symphony::internal::cldevice *d_ptr = ::symphony::internal::c_ptr(::symphony::internal::s_dev_ptrs->at(0));
  SYMPHONY_INTERNAL_ASSERT(d_ptr != nullptr, "null device_ptr");
  symphony::internal::legacy::device_ptr device = (*::symphony::internal::s_dev_ptrs)[0];
#else

  symphony::internal::legacy::device_ptr device;
#endif

  return device;
}

template<size_t Dims, typename Body>
inline ::symphony::internal::task* api20_create_ndrange_task(const symphony::range<Dims>& global_range,
                                                         const symphony::range<Dims>& local_range,
                                                         Body&& body)
{
  typedef ::symphony::internal::legacy::body_wrapper<Body> wrapper;
  auto device = get_gpu_device();

  for(auto i : global_range.stride())
    SYMPHONY_API_ASSERT( (i== 1), "GPU global ranges must be unit stride");
  for(auto i : local_range.stride())
    SYMPHONY_API_ASSERT( (i== 1), "GPU local ranges must be unit stride");
  ::symphony::internal::task* t = wrapper::create_task(device,
                                                   global_range,
                                                   local_range,
                                                   std::forward<Body>(body));

  return t;
}

template<size_t Dims, typename Body>
::symphony::internal::task* api20_create_ndrange_task(const symphony::range<Dims>& global_range,
                                                         Body&& body)
{
  symphony::range<Dims> local_range;
  return api20_create_ndrange_task(global_range, local_range,
                                   std::forward<Body>(body));
}

template<typename ...Args>
class gpu_kernel_implementation
{

  ::symphony::internal::legacy::kernel_ptr<Args...>  _kernel;

  template<typename Code, class Enable>
  friend
  struct ::symphony::internal::task_factory_dispatch;

  template<typename GPUKernel, typename ...CallArgs>
  friend
  void symphony::internal::execute_gpu(GPUKernel const& gk, CallArgs&&... args);

  template<typename GPUKernel, size_t Dims, typename ...CallArgs>
  friend
  struct symphony::internal::executor;

  template<typename BAS, typename Executor>
  friend
  struct symphony::internal::process_executor_buffers;

public:
  using params_tuple = std::tuple<Args...>;

protected:
  gpu_kernel_implementation(std::string const& kernel_str,
                          std::string const& kernel_name,
                          std::string const& build_options) :
    _kernel(::symphony::internal::legacy::create_kernel<Args...>(kernel_str, kernel_name, build_options))
  {}

  explicit gpu_kernel_implementation(std::string const& gl_kernel_str) :
    _kernel(::symphony::internal::legacy::create_kernel<Args...>(gl_kernel_str))
  {}

  gpu_kernel_implementation(void const* kernel_bin,
                            size_t kernel_len,
                            std::string const& kernel_name,
                            std::string const& build_options) :
    _kernel(::symphony::internal::legacy::create_kernel<Args...>(kernel_bin,
                                                             kernel_len,
                                                             kernel_name,
                                                             build_options))
  {}

#ifdef SYMPHONY_HAVE_OPENCL
  std::pair<void const*, size_t> get_cl_kernel_binary() const
  {
    auto k_ptr = internal::c_ptr(_kernel);
    return k_ptr->get_cl_kernel_binary();
  }
#endif

  SYMPHONY_DEFAULT_METHOD(gpu_kernel_implementation(gpu_kernel_implementation const&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel_implementation& operator=(gpu_kernel_implementation const&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel_implementation(gpu_kernel_implementation&&));
  SYMPHONY_DEFAULT_METHOD(gpu_kernel_implementation& operator=(gpu_kernel_implementation&&));

  template<typename TaskParam1, typename ...TaskParams>
  symphony::internal::task*
  create_task_helper_with_global_range(TaskParam1 const& global_range,
                                       TaskParams&&... task_params) const
  {
    auto attrs = symphony::internal::legacy::create_task_attrs(symphony::internal::legacy::attr::gpu);
    auto gpu_task = api20_create_ndrange_task(global_range,
                                              symphony::internal::legacy::with_attrs(attrs,
                                                               _kernel,
                                                               std::forward<TaskParams>(task_params)...));
    return gpu_task;
  }

  template<typename TaskParam1, typename TaskParam2, typename ...TaskParams>
  symphony::internal::task*
  create_task_helper_with_global_and_local_ranges(TaskParam1 const& global_range,
                                                  TaskParam2 const& local_range,
                                                  TaskParams&&... task_params) const
  {
    auto attrs = symphony::internal::legacy::create_task_attrs(symphony::internal::legacy::attr::gpu);
    auto gpu_task = api20_create_ndrange_task(global_range,
                                              local_range,
                                              symphony::internal::legacy::with_attrs(attrs,
                                                               _kernel,
                                                               std::forward<TaskParams>(task_params)...));
    return gpu_task;
  }

  template<typename BAS, size_t Dims, typename ...TaskParams>
  void
  execute_with_global_and_local_ranges(executor_construct const& exec_cons,
                                       BAS& bas,
                                       bool add_buffers,
                                       bool perform_launch,
                                       ::symphony::range<Dims> const& global_range,
                                       ::symphony::range<Dims> const& local_range,
                                       TaskParams&&... task_params) const
  {
    auto device = get_gpu_device();

    for(auto i : global_range.stride())
      SYMPHONY_API_ASSERT( (i== 1), "GPU global ranges must be unit stride");
    for(auto i : local_range.stride())
      SYMPHONY_API_ASSERT( (i== 1), "GPU local ranges must be unit stride");

    gpu_launch_info gli(device);

    using args_tuple   = std::tuple< typename strip_toplevel<TaskParams>::type... >;

    auto args = std::make_tuple( std::forward<TaskParams>(task_params)... );
    normal_args_container<params_tuple, args_tuple> my_normal_args_container(args);

    bool first_execution = true;

    auto k_ptr = internal::c_ptr(_kernel);
    bool conflict = true;
    size_t spin_count = 10;
    while(conflict) {
      if(spin_count > 0)
        spin_count--;
      else
        usleep(1);

      conflict = !gpu_do_execute<Dims,
                                 params_tuple,
                                 args_tuple,
                                 decltype(k_ptr),
                                 decltype(my_normal_args_container._tp),
                                 decltype(bas),
                                 preacquired_arenas_base>
                                (k_ptr,
                                 exec_cons,
                                 gli,
                                 device,
                                 global_range,
                                 local_range,
                                 std::make_tuple( std::forward<TaskParams>(task_params)... ),
                                 my_normal_args_container._tp,
                                 bas,
                                 nullptr,
                                 first_execution && add_buffers,
                                 perform_launch);

      SYMPHONY_INTERNAL_ASSERT(perform_launch || !conflict,
                               "Conflict not expected when launch is not attempted");
      first_execution = false;
    }
  }

  template<typename BAS, size_t Dims, typename ...TaskParams>
  void
  execute_with_global_range(executor_construct const& exec_cons,
                            BAS& bas,
                            bool add_buffers,
                            bool perform_launch,
                            ::symphony::range<Dims> const& global_range,
                            TaskParams&&... task_params) const
  {
    ::symphony::range<Dims> local_range;
    execute_with_global_and_local_ranges(exec_cons,
                                         bas,
                                         add_buffers,
                                         perform_launch,
                                         global_range,
                                         local_range,
                                         std::forward<TaskParams>(task_params)...);
  }

  ~gpu_kernel_implementation()
  {}
};

};
};

#endif
