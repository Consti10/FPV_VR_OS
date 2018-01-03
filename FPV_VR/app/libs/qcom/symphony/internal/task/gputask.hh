// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef SYMPHONY_HAVE_GPU

#include <symphony/buffer.hh>
#include <symphony/internal/legacy/gpukernel.hh>
#include <symphony/internal/buffer/arenaaccess.hh>
#include <symphony/internal/buffer/bufferpolicy.hh>
#include <symphony/internal/device/cldevice.hh>
#include <symphony/internal/device/clevent.hh>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/strprintf.hh>

namespace symphony {

template <typename T>
struct buffer_attrs {
  typedef T type;
};

template<typename T>
class local {};

namespace internal {

class task_bundle_dispatch;

class gputask_base : public task {
protected:
  using dyn_sized_bas = buffer_acquire_set<0, false>;

  bool _executed;

  bool _does_bundle_dispatch;

  bool _first_execution;

  uint64_t _exec_time;

  virtual void release_arguments() = 0;
public:
  explicit gputask_base(legacy::task_attrs a) :
    task(nullptr, a)
    , _executed(false)
    , _does_bundle_dispatch(false)
    , _first_execution(true)
    , _exec_time(0)
  {}

  void configure_for_bundle_dispatch() {
    _does_bundle_dispatch = true;
  }

  virtual void update_buffer_acquire_set(dyn_sized_bas& bas) = 0;

  bool has_executed() const { return _executed; }

  virtual executor_device get_executor_device() const = 0;

  virtual bool do_execute(task_bundle_dispatch* tbd = nullptr) = 0;

  uint64_t get_exec_time() const {
    return _exec_time;
  }

  void set_exec_time(uint64_t elapsed_time) {
    _exec_time = elapsed_time;
  }

  void on_finish() {

    if (_exec_time != 0) {
      uint64_t elapsed_time = symphony_get_time_now() - _exec_time;
      set_exec_time(elapsed_time);
    }

    if(!_does_bundle_dispatch)
      release_arguments();
  }
};

namespace gpu_kernel_dispatch {

template<typename T>
struct cl_arg_setter
{
  template<typename Kernel>
  static void set(Kernel* kernel,
                  size_t index,
                  T& arg) {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: value type", index);
    kernel->set_arg(index, arg);
  }
};

struct cl_arg_local_alloc_setter
{
  template<typename Kernel>
  static void set(Kernel* kernel,
                  size_t index,
                  size_t num_bytes_to_local_alloc) {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: local_alloc %zu bytes", index, num_bytes_to_local_alloc);
    kernel->set_arg_local_alloc(index, num_bytes_to_local_alloc);
  }
};

struct input_output_param{};
struct input_param{};
struct output_param{};
struct const_buffer{};
struct non_const_buffer{};

struct cl_arg_api20_buffer_setter
{
#ifdef SYMPHONY_HAVE_OPENCL
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::clkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  input_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr in", i);

    SYMPHONY_API_ASSERT(b != nullptr,
                    "Need non-null buffer_ptr to execute with gpu kernel");

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

#ifdef SYMPHONY_CL_TO_CL2
    auto svm_ptr = arena_storage_accessor::access_cl2_arena_for_gputask(acquired_arena);
    kernel->set_arg_svm(i, svm_ptr);
#else
    auto ocl_buffer = arena_storage_accessor::access_cl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, ocl_buffer);
#endif
  }
#endif

#ifdef SYMPHONY_HAVE_GLES
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::glkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  input_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr in", i);

    SYMPHONY_API_ASSERT(b != nullptr,
                    "Need non-null buffer_ptr to execute with gpu kernel");

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

    auto id = arena_storage_accessor::access_gl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, id);
  }
#endif

#ifdef SYMPHONY_HAVE_OPENCL
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::clkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  output_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr out", i);

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

#ifdef SYMPHONY_CL_TO_CL2
    auto svm_ptr = arena_storage_accessor::access_cl2_arena_for_gputask(acquired_arena);
    kernel->set_arg_svm(i, svm_ptr);
#else
    auto ocl_buffer = arena_storage_accessor::access_cl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, ocl_buffer);
#endif
  }
#endif

#ifdef SYMPHONY_HAVE_GLES
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::glkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  output_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr out", i);

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

    auto id = arena_storage_accessor::access_gl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, id);
  }
#endif

#ifdef SYMPHONY_HAVE_OPENCL
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::clkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  input_output_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr inout", i);

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

#ifdef SYMPHONY_CL_TO_CL2
    auto svm_ptr = arena_storage_accessor::access_cl2_arena_for_gputask(acquired_arena);
    kernel->set_arg_svm(i, svm_ptr);
#else
    auto ocl_buffer = arena_storage_accessor::access_cl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, ocl_buffer);
#endif
  }
#endif

#ifdef SYMPHONY_HAVE_GLES
  template<typename BufferPtr, typename BufferAcquireSet>
  static void set(::symphony::internal::legacy::device_ptr const& device,
                  internal::glkernel* kernel,
                  size_t i,
                  BufferPtr& b,
                  input_output_param,
                  BufferAcquireSet& bas)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: symphony::buffer_ptr inout", i);

    SYMPHONY_UNUSED(device);

    auto acquired_arena = bas.find_acquired_arena(b);

    auto id = arena_storage_accessor::access_gl_arena_for_gputask(acquired_arena);
    kernel->set_arg(i, id);
  }
#endif

};

struct cl_arg_texture_setter {
#ifdef SYMPHONY_HAVE_OPENCL
  static void set(::symphony::internal::legacy::device_ptr const&,
                  internal::clkernel* kernel,
                  size_t i,
                  symphony::graphics::internal::base_texture_cl* tex_cl)
  {
    SYMPHONY_INTERNAL_ASSERT(tex_cl != nullptr, "Null texture_cl");
    SYMPHONY_DLOG("Setting kernel argument[%zu]: texture type. Parameter: in_out",
              i);
    kernel->set_arg(i, tex_cl);
  }
#endif

#if defined(SYMPHONY_HAVE_OPENCL) && defined(SYMPHONY_HAVE_GLES)
  static void set(::symphony::internal::legacy::device_ptr const&,
                  internal::glkernel* ,
                  size_t ,
                  symphony::graphics::internal::base_texture_cl* )
  {
    SYMPHONY_UNIMPLEMENTED("Textures not implemented in GL compute path");
  }
#endif

};

struct cl_arg_sampler_setter {
#ifdef SYMPHONY_HAVE_OPENCL
  template<typename SamplerPtr>
  static void set(::symphony::internal::legacy::device_ptr const&,
                  internal::clkernel* kernel,
                  size_t i,
                  SamplerPtr& sampler_ptr)
  {
    SYMPHONY_DLOG("Setting kernel argument[%zu]: sampler type. Parameter: in_out",
              i);
    kernel->set_arg(i, sampler_ptr);
  }
#endif

#ifdef SYMPHONY_HAVE_GLES
  template<typename SamplerPtr>
  static void set(::symphony::internal::legacy::device_ptr const&,
                  internal::glkernel* ,
                  size_t ,
                  SamplerPtr& )
  {
    SYMPHONY_UNIMPLEMENTED("Samplers not implemented in GL compute path");
  }
#endif

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename T>
struct is_local_alloc: public std::false_type
{
  typedef void data_type;
};

template<typename U>
struct is_local_alloc<symphony::local<U>>: public std::true_type
{
  typedef U data_type;
};

template<typename T>
struct is_texture_ptr : public std::false_type {};

#ifdef SYMPHONY_HAVE_OPENCL
template <symphony::graphics::image_format ImageFormat, int Dims>
struct is_texture_ptr<symphony::graphics::texture_ptr<ImageFormat, Dims>> : public std::true_type {};
#endif

template<typename T>
struct is_sampler_ptr : public std::false_type {};

#ifdef SYMPHONY_HAVE_OPENCL
template <symphony::graphics::addressing_mode addrMode, symphony::graphics::filter_mode filterMode>
struct is_sampler_ptr<symphony::graphics::sampler_ptr<addrMode, filterMode>> : public std::true_type {};
#endif

template<typename T>
struct is_normal_arg : public std::conditional<!is_api20_buffer_ptr<T>::value &&
                                               !is_local_alloc<T>::value &&
                                               !is_texture_ptr<T>::value &&
                                               !is_sampler_ptr<T>::value,
                                               std::true_type,
                                               std::false_type>::type
{};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename T> struct kernel_arg_ptr_traits;

template<typename T>
struct kernel_arg_ptr_traits< ::symphony::buffer_ptr<T> > {
  using naked_type = typename std::remove_cv<T>::type;
  using is_const   = typename std::is_const<T>;
};

template<typename T> struct kernel_param_ptr_traits;

template<typename T>
struct kernel_param_ptr_traits<symphony::buffer_ptr<T>> {
  using naked_type = typename std::remove_cv<T>::type;
  using is_const  = std::is_const<T>;
  using direction = typename std::conditional<is_const::value, input_param, input_output_param>::type;
};

template<typename T>
struct kernel_param_ptr_traits<symphony::in<symphony::buffer_ptr<T>>> {
  using naked_type = typename std::remove_cv<T>::type;
  using is_const  = std::is_const<T>;
  using direction = input_param;
};

template<typename T>
struct kernel_param_ptr_traits<symphony::out<symphony::buffer_ptr<T>>> {
  using naked_type = typename std::remove_cv<T>::type;
  using is_const  = std::is_const<T>;
  using direction = output_param;
};

template<typename T>
struct kernel_param_ptr_traits<symphony::inout<symphony::buffer_ptr<T>>> {
  using naked_type = typename std::remove_cv<T>::type;
  using is_const  = std::is_const<T>;
  using direction = input_output_param;
};

#ifdef SYMPHONY_HAVE_OPENCL

template<typename Param, typename Arg, typename Kernel>
void parse_normal_arg(::symphony::internal::legacy::device_ptr const&,
                      Kernel* kernel,
                      size_t index,
                      Arg& arg,
                      bool do_not_dispatch)
{

  static_assert(!is_api20_buffer_ptr<Param>::value, "Expected buffer_ptr argument");
  static_assert(!is_texture_ptr<Param>::value, "Expected texture_ptr or buffer_ptr argument");
  static_assert(std::is_same<Param, Arg>::value, "Mismatch in type of normal argument");
  if(do_not_dispatch == false) {
    cl_arg_setter<Arg>::set(kernel, index, arg);
  }
}
#endif

template<typename Param, typename Arg, typename Kernel>
void parse_local_alloc_arg(::symphony::internal::legacy::device_ptr const&,
                           Kernel* kernel,
                           size_t index,
                           Arg& arg,
                           bool do_not_dispatch)
{
  static_assert(is_local_alloc<Param>::value, "Expected a symphony::local parameter");
  size_t sizeof_T = sizeof(typename is_local_alloc<Param>::data_type);

  size_t num_bytes_to_local_alloc = sizeof_T * arg;
  if(do_not_dispatch == false) {
    cl_arg_local_alloc_setter::set(kernel, index, num_bytes_to_local_alloc);
  }
}

template<typename Param, typename Arg, typename BufferAcquireSet, typename Kernel>
void parse_api20_buffer_ptr_arg_dispatch(std::true_type,
                                         ::symphony::internal::legacy::device_ptr const& device,
                                         Kernel* kernel,
                                         size_t index,
                                         Arg& arg,
                                         BufferAcquireSet& bas,
                                         bool do_not_dispatch)
{
  static_assert(symphony::internal::is_api20_buffer_ptr<Param>::value, "Param must be a symphony::buffer_ptr");

  typedef kernel_param_ptr_traits<Param> param_traits;
  using direction = typename param_traits::direction;

  typedef kernel_arg_ptr_traits<Arg> arg_traits;

  static_assert(std::is_same<typename param_traits::naked_type, typename arg_traits::naked_type>::value == true,
                "Incompatible symphony::buffer_ptr data types");

  static_assert(param_traits::is_const::value == false || std::is_same<direction, input_param>::value == true,
                "buffer_ptr of const data type may only be declared as input to kernel");

  static_assert(arg_traits::is_const::value == false || std::is_same<direction, input_param>::value == true,
                "A buffer_ptr<const T> argument may only be passed as a kernel input");

  if(do_not_dispatch == true) {

    auto acquire_action = (std::is_same<direction, input_param>::value == true ? bufferpolicy::acquire_r :
                           (std::is_same<direction, output_param>::value == true ? bufferpolicy::acquire_w :
                                                                                   bufferpolicy::acquire_rw));
    bas.add(arg, acquire_action);
  }
  else {

    cl_arg_api20_buffer_setter::set(device,
                                    kernel,
                                    index,
                                    arg,
                                    direction(),
                                    bas);
  }
}

#ifdef SYMPHONY_HAVE_OPENCL

template<typename Param, typename Arg, typename BufferAcquireSet, typename Kernel>
void parse_api20_buffer_ptr_arg_dispatch(std::false_type,
                                         ::symphony::internal::legacy::device_ptr const& device,
                                         Kernel* kernel,
                                         size_t index,
                                         Arg& arg,
                                         BufferAcquireSet& bas,
                                         bool do_not_dispatch)
{
  static_assert(is_texture_ptr<Param>::value, "Param must be a symphony::texture_ptr");

  if(do_not_dispatch == true) {

    auto acquire_action = bufferpolicy::acquire_rw;

    auto& bb = reinterpret_cast<symphony::internal::buffer_ptr_base&>(arg);
    auto buf_as_tex_info = buffer_accessor::get_buffer_as_texture_info(bb);

    using tpinfo = symphony::graphics::internal::texture_ptr_info<Param>;
    SYMPHONY_API_ASSERT(buf_as_tex_info.get_used_as_texture(),
                    "buffer_ptr is not setup to be treated as texture");
    SYMPHONY_INTERNAL_ASSERT(buf_as_tex_info.get_dims() > 0 && buf_as_tex_info.get_dims() <= 3,
                         "buffer_ptr treated as texture of invalid dimension");
    SYMPHONY_API_ASSERT(buf_as_tex_info.get_dims() == tpinfo::dims,
                    "buffer_ptr is setup to be interpreted as %d dimensional, but kernel requires %d dimensions",
                    buf_as_tex_info.get_dims(),
                    tpinfo::dims);

    auto bufstate = c_ptr(buffer_accessor::get_bufstate(bb));
    SYMPHONY_INTERNAL_ASSERT(bufstate != nullptr, "Null bufstate");
    auto preexisting_tex_a = bufstate->get(TEXTURE_ARENA);
    if(preexisting_tex_a == nullptr || !arena_storage_accessor::texture_arena_has_format(preexisting_tex_a)) {

      SYMPHONY_DLOG("Allocating new texture to save in texture_arena of bufstate=%p", bufstate);
      bb.allocate_host_accessible_data(true);
      bool has_ion_ptr = false;
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
      auto preexisting_ion_a = bufstate->get(ION_ARENA);
      has_ion_ptr = (preexisting_ion_a != nullptr &&
                     arg.host_data() == arena_storage_accessor::get_ptr(preexisting_ion_a));
#endif
      auto tex_cl = new symphony::graphics::internal::texture_cl
                                     <tpinfo::img_format, tpinfo::dims>
                                     (symphony::graphics::internal::image_size_converter<tpinfo::dims, 3>::
                                                                    convert(buf_as_tex_info.get_img_size()),
                                      false,
                                      arg.host_data(),
                                      has_ion_ptr,
                                      false);
      SYMPHONY_INTERNAL_ASSERT(tex_cl != nullptr,
                           "internal texture_cl creation failed for bufferstate=%p",
                           ::symphony::internal::c_ptr(buffer_accessor::get_bufstate(bb)));
      SYMPHONY_DLOG("Created tex_cl=%p for bufstate=%p",
                tex_cl,
                ::symphony::internal::c_ptr(buffer_accessor::get_bufstate(bb)));
      buf_as_tex_info.access_tex_cl() = tex_cl;
    }

    bas.add(arg, acquire_action, buf_as_tex_info);
  }
  else {

    auto acquired_arena = bas.find_acquired_arena(arg);

    auto tex_cl = arena_storage_accessor::access_texture_arena_for_gputask(acquired_arena);
    cl_arg_texture_setter::set(device,
                               kernel,
                               index,
                               tex_cl);
  }
}
#endif

template<typename Param, typename Arg, typename BufferAcquireSet, typename Kernel>
void parse_api20_buffer_ptr_arg(::symphony::internal::legacy::device_ptr const& device,
                                Kernel* kernel,
                                size_t index,
                                Arg& arg,
                                BufferAcquireSet& bas,
                                bool do_not_dispatch)
{
  static_assert(symphony::internal::is_api20_buffer_ptr<Param>::value || is_texture_ptr<Param>::value,
                "Unexpected symphony::buffer_ptr argument");

  parse_api20_buffer_ptr_arg_dispatch<Param, Arg, BufferAcquireSet>
                                     (symphony::internal::is_api20_buffer_ptr<Param>(),
                                      device,
                                      kernel,
                                      index,
                                      arg,
                                      bas,
                                      do_not_dispatch);

}

#ifdef SYMPHONY_HAVE_OPENCL

template<typename Param, typename Arg, typename Kernel>
void parse_texture_ptr_arg(::symphony::internal::legacy::device_ptr const& device,
                          Kernel* kernel,
                          size_t index,
                          Arg& arg,
                          bool do_not_dispatch)
{

  static_assert(is_texture_ptr<Param>::value, "Unexpected texture_ptr argument");

  static_assert(std::is_same<Param, Arg>::value,
                "Incompatible texture_ptr types");

  if(do_not_dispatch == false) {
    cl_arg_texture_setter::set(device,
                               kernel,
                               index,
                               internal::c_ptr(arg));
  }
}
#endif

#ifdef SYMPHONY_HAVE_OPENCL

template<typename Param, typename Arg, typename Kernel>
void parse_sampler_ptr_arg(::symphony::internal::legacy::device_ptr const& device,
                          Kernel* kernel,
                          size_t index,
                          Arg& arg,
                          bool do_not_dispatch)
{

  static_assert(is_sampler_ptr<Param>::value, "Expected sampler_ptr argument");

  static_assert(std::is_same<Param, Arg>::value,
                "Incompatible sampler_ptr types");

  if(do_not_dispatch == false) {
    cl_arg_sampler_setter::set(device,
                               kernel,
                               index,
                               arg);
  }
}
#endif

enum class dispatch_type
{
  normal,
  local_alloc,
  api20_buffer,
  texture,
  sampler
};

template<bool is_arg_a_local_alloc,
         bool is_arg_an_api20_buffer_ptr,
         bool is_arg_a_texture_ptr,
         bool is_arg_a_sampler_ptr>
struct translate_flags_to_dispatch_type;

template<>
struct translate_flags_to_dispatch_type<false, false, false, false>
{
  static auto const value = dispatch_type::normal;
};

template<>
struct translate_flags_to_dispatch_type<true, false, false, false>
{
  static auto const value = dispatch_type::local_alloc;
};

template<>
struct translate_flags_to_dispatch_type<false, true, false, false>
{
  static auto const value = dispatch_type::api20_buffer;
};

template<>
struct translate_flags_to_dispatch_type<false, false, true, false>
{
  static auto const value = dispatch_type::texture;
};

template<>
struct translate_flags_to_dispatch_type<false, false, false, true>
{
  static auto const value = dispatch_type::sampler;
};

#ifdef SYMPHONY_HAVE_OPENCL

template<typename Param, typename Arg, dispatch_type DispatchType, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<DispatchType == dispatch_type::normal, void>::type
parse_arg_dispatch(::symphony::internal::legacy::device_ptr const& device,
                        Kernel* kernel,
                        size_t index,
                        Arg& arg,
                        BufferAcquireSet&,
                        bool do_not_dispatch)
{
  parse_normal_arg<Param, Arg>(device, kernel, index, arg, do_not_dispatch);
}
#endif

#ifdef SYMPHONY_HAVE_OPENCL
template<typename Param, typename Arg, dispatch_type DispatchType, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<DispatchType == dispatch_type::local_alloc, void>::type
parse_arg_dispatch(::symphony::internal::legacy::device_ptr const& device,
                        Kernel* kernel,
                        size_t index,
                        Arg& arg,
                        BufferAcquireSet&,
                        bool do_not_dispatch)
{
  parse_local_alloc_arg<Param, Arg>(device, kernel, index, arg, do_not_dispatch);
}
#endif

template<typename Param, typename Arg, dispatch_type DispatchType, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<DispatchType == dispatch_type::api20_buffer, void>::type
parse_arg_dispatch(::symphony::internal::legacy::device_ptr const& device,
                        Kernel* kernel,
                        size_t index,
                        Arg& arg,
                        BufferAcquireSet& bas,
                        bool do_not_dispatch)
{
  parse_api20_buffer_ptr_arg<Param, Arg>(device,
                                         kernel,
                                         index,
                                         arg,
                                         bas,
                                         do_not_dispatch);
}

#ifdef SYMPHONY_HAVE_OPENCL
template<typename Param, typename Arg, dispatch_type DispatchType, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<DispatchType == dispatch_type::texture, void>::type
parse_arg_dispatch(::symphony::internal::legacy::device_ptr const& device,
                        Kernel* kernel,
                        size_t index,
                        Arg& arg,
                        BufferAcquireSet&,
                        bool do_not_dispatch)
{
  parse_texture_ptr_arg<Param, Arg>(device, kernel, index, arg, do_not_dispatch);
}
#endif

#ifdef SYMPHONY_HAVE_OPENCL
template<typename Param, typename Arg, dispatch_type DispatchType, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<DispatchType == dispatch_type::sampler, void>::type
parse_arg_dispatch(::symphony::internal::legacy::device_ptr const& device,
                        Kernel* kernel,
                        size_t index,
                        Arg& arg,
                        BufferAcquireSet&,
                        bool do_not_dispatch)
{
  parse_sampler_ptr_arg<Param, Arg>(device, kernel, index, arg, do_not_dispatch);
}
#endif

template<typename Params, typename Args, typename NormalArgs, size_t index = 0, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<index == std::tuple_size<Args>::value, void>::type
prepare_args_pass(::symphony::internal::legacy::device_ptr const&,
                  Kernel*,
                  Args&&,
                  NormalArgs&,
                  BufferAcquireSet&,
                  bool)
{

}

template<typename Norm, typename Arg, bool IsParamNormal>
struct select_normal_arg {
  static Arg& get(Norm&, Arg& arg) { return arg; }
};

template<typename Norm, typename Arg>
struct select_normal_arg<Norm, Arg, true> {
  static Norm& get(Norm& norm, Arg&) { return norm; }
};

template<typename Params, typename Args, typename NormalArgs, size_t index = 0, typename BufferAcquireSet, typename Kernel>
typename std::enable_if<index < std::tuple_size<Args>::value, void>::type
prepare_args_pass(::symphony::internal::legacy::device_ptr const& device,
                  Kernel* kernel,
                  Args&& args,
                  NormalArgs& normal_args,
                  BufferAcquireSet& bas,
                  bool do_not_dispatch)
{

  static_assert(std::tuple_size<Args>::value == std::tuple_size<Params>::value,
                "Number of parameters is different to number of arguments");

  typedef typename std::tuple_element<index, Params>::type param_type;
  typedef typename std::tuple_element<index, Args>::type arg_type;
  typedef typename std::tuple_element<index, NormalArgs>::type norm_type;

  typedef typename is_local_alloc<param_type>::type is_param_a_local_alloc;
  typedef typename symphony::internal::is_api20_buffer_ptr<arg_type>::type is_arg_an_api20_buffer_ptr;
  typedef typename is_texture_ptr<arg_type>::type is_arg_a_texture_ptr;
  typedef typename is_sampler_ptr<arg_type>::type is_arg_a_sampler_ptr;

  auto constexpr is_norm = is_normal_arg<param_type>::value;
  using param_type_to_pass = typename std::conditional<is_norm, typename std::remove_cv<param_type>::type, param_type>::type;
  using arg_type_to_pass = typename std::conditional<is_norm, norm_type, arg_type>::type;
  auto& arg_to_pass = select_normal_arg<norm_type, arg_type, is_norm>::get(std::get<index>(normal_args), std::get<index>(args));

  parse_arg_dispatch<param_type_to_pass,
                     arg_type_to_pass,
                     translate_flags_to_dispatch_type<is_param_a_local_alloc::value,
                                                      is_arg_an_api20_buffer_ptr::value,
                                                      is_arg_a_texture_ptr::value,
                                                      is_arg_a_sampler_ptr::value>::value>
                    (device,
                     kernel,
                     index,
                     arg_to_pass,
                     bas,
                     do_not_dispatch);

  prepare_args_pass<Params, Args, NormalArgs, index + 1>
                   (device,
                    kernel,
                    std::forward<Args>(args),
                    normal_args,
                    bas,
                    do_not_dispatch);
}

template<typename Params,
         typename Args,
         typename NormalArgs,
         typename BufferAcquireSet,
         typename PreacquiredArenas,
         typename Kernel>
bool
prepare_args(::symphony::internal::legacy::device_ptr const& device,
             Kernel* k_ptr,
             Args&& args,
             NormalArgs& normal_args,
             BufferAcquireSet& bas,
             PreacquiredArenas const* p_preacquired_arenas,
             void const* requestor,
             bool add_buffers = true,
             bool acquire_buffers = true,
             bool dispatch_args = true)
{
  SYMPHONY_INTERNAL_ASSERT(k_ptr != nullptr, "null kernel");

  if (add_buffers) {

    if(0) {}
#ifdef SYMPHONY_HAVE_OPENCL
    else if(k_ptr->is_cl())
      prepare_args_pass<Params, Args>
                       (device,
                        k_ptr->get_cl_kernel(),
                        std::forward<Args>(args),
                        normal_args,
                        bas,
                        true);
#endif
#ifdef SYMPHONY_HAVE_GLES
    else if(k_ptr->is_gl())
      prepare_args_pass<Params, Args>
                       (device,
                        k_ptr->get_gl_kernel(),
                        std::forward<Args>(args),
                        normal_args,
                        bas,
                        true);
#endif
    else
      SYMPHONY_FATAL("Unsupported type of GPU kernel");
  }

  if(acquire_buffers) {
    symphony::internal::executor_device ed = symphony::internal::executor_device::unspecified;
#ifdef SYMPHONY_HAVE_OPENCL
    if(k_ptr->is_cl())
      ed = symphony::internal::executor_device::gpucl;
#endif
#ifdef SYMPHONY_HAVE_GLES
    if(k_ptr->is_gl())
      ed = symphony::internal::executor_device::gpugl;
#endif

    bas.acquire_buffers(requestor,
                        {ed},
                        true,
                        (p_preacquired_arenas != nullptr && p_preacquired_arenas->has_any() ?
                         p_preacquired_arenas : nullptr));
    if (!bas.acquired()) {
      return false;
    }
  }

  if(dispatch_args) {

    if(0) {}
#ifdef SYMPHONY_HAVE_OPENCL
    else if(k_ptr->is_cl())
      prepare_args_pass<Params, Args>
                       (device,
                        k_ptr->get_cl_kernel(),
                        std::forward<Args>(args),
                        normal_args,
                        bas,
                        false);
#endif
#ifdef SYMPHONY_HAVE_GLES
    else if(k_ptr->is_gl())
      prepare_args_pass<Params, Args>
                       (device,
                        k_ptr->get_gl_kernel(),
                        std::forward<Args>(args),
                        normal_args,
                        bas,
                        false);
#endif
    else
      SYMPHONY_FATAL("Unsupported type of GPU kernel");
  }

  return true;
}

};

#ifdef SYMPHONY_HAVE_OPENCL
void CL_CALLBACK task_bundle_completion_callback(cl_event event, cl_int, void* user_data);
#endif

class task_bundle_dispatch {

  std::vector<gputask_base*> _tasks;

  unsigned int _depth;

  using dyn_sized_bas = buffer_acquire_set<0, false>;
  using dyn_sized_pa  = preacquired_arenas<false>;

  dyn_sized_bas _bas;
  dyn_sized_pa  _preacquired_arenas;

  void add(task* t, bool first = false) {
    SYMPHONY_DLOG("task_bundle_dispatch() %p: adding gputask %p", this, t);
    SYMPHONY_INTERNAL_ASSERT(t != nullptr, "Cannot add a null task to task_bundle_dispatch");
    auto gt = static_cast<gputask_base*>(t);
    SYMPHONY_INTERNAL_ASSERT(gt != nullptr, "task_bundle_dispatch: received a non-GPU task");
    if (!first) {
      _tasks.push_back(gt);
    } else {
      SYMPHONY_INTERNAL_ASSERT(_tasks[0] == nullptr,
          "first element must be written exactly once");
      _tasks[0] = gt;
    }
    gt->configure_for_bundle_dispatch();
    gt->update_buffer_acquire_set(_bas);
  }

  void add_root(task* t) {
    add(t, true);
  }

public:

  task_bundle_dispatch() :
    _tasks(),
    _depth(0),
    _bas(),
    _preacquired_arenas()
  {
    _tasks.reserve(8);
    _tasks.push_back(nullptr);

  }

  void add_non_root(task* t) {
    add(t, false);
  }

  void execute_all(task* root_task) {
    SYMPHONY_INTERNAL_ASSERT(_tasks.size() > 0,
                        "task_bundle_dispatch should be called on bundles of"
                        "at least two tasks (including the root_task)");
    SYMPHONY_INTERNAL_ASSERT(_depth == 0,
                        "Can call execute_all only at scheduling depth of zero");

    add_root(root_task);

    SYMPHONY_DLOG("task_bundle_dispatch %p: execute_all()", this);

    SYMPHONY_INTERNAL_ASSERT(root_task != nullptr, "root task is null");
    void const* requestor = root_task;

    auto gpu_root_task = static_cast<gputask_base*>(root_task);
    SYMPHONY_INTERNAL_ASSERT(gpu_root_task != nullptr, "Current limitation: root task must be a gputask.");
    auto ed = gpu_root_task->get_executor_device();

    _bas.blocking_acquire_buffers(requestor,
                                  {ed},
                                  _preacquired_arenas.has_any() ? &_preacquired_arenas : nullptr);
    for(auto& gt : _tasks) {
      SYMPHONY_INTERNAL_ASSERT(gt != nullptr, "Null gputask found during bundle dispatch");
      SYMPHONY_INTERNAL_ASSERT(gt->get_executor_device() == ed, "Mismatch in executor device type in bundle");
      gt->execute_sync(this);
    }
  }

  std::vector<gputask_base*> const& get_tasks() { return _tasks; }

  void increment_depth() { _depth++; }

  void decrement_depth() { _depth--; }

  unsigned int get_depth() const { return _depth; }

  bool contains_many() const { return _tasks.size() > 1; }

  dyn_sized_bas& get_buffer_acquire_set() { return _bas; }

  dyn_sized_pa& get_preacquired_arenas() { return _preacquired_arenas; }

  void register_preacquired_arena(bufferstate* bufstate,
                                  arena* preacquired_arena)
  {
    _preacquired_arenas.register_preacquired_arena(bufstate, preacquired_arena);
  }
};

#ifdef SYMPHONY_HAVE_OPENCL
void CL_CALLBACK completion_callback(cl_event event, cl_int, void* user_data);
#endif

template<typename TupledParams,
         size_t   index = std::tuple_size<TupledParams>::value,
         typename ...ExtractedKernelArgs>
struct kernel_ptr_for_tupled_params
{
  using type = typename kernel_ptr_for_tupled_params<TupledParams,
                                                     index-1,
                                                     typename std::tuple_element<index-1, TupledParams>::type,
                                                     ExtractedKernelArgs...>::type;
};

template<typename TupledParams, typename ...ExtractedKernelArgs>
struct kernel_ptr_for_tupled_params<TupledParams, 0, ExtractedKernelArgs...>
{
  using type = ::symphony::internal::legacy::kernel_ptr<ExtractedKernelArgs...>;
};

template<typename Params>
struct normal_args_tuple_type;

template<typename ...Ts>
struct normal_args_tuple_type<std::tuple<Ts...>> {
  using type = std::tuple< typename std::conditional<gpu_kernel_dispatch::is_normal_arg<Ts>::value,
                                                     typename std::remove_cv<Ts>::type,
                                                     char>::type ... >;
};

template<typename Params, typename Args, size_t Index, bool IsNormal>
struct normal_args_tuple_single_copier {
  normal_args_tuple_single_copier(Args&, typename normal_args_tuple_type<Params>::type&) {

  }
};

template<typename Params, typename Args, size_t Index>
struct normal_args_tuple_single_copier<Params, Args, Index, true> {
  normal_args_tuple_single_copier(Args& args, typename normal_args_tuple_type<Params>::type& normal_args) {
    std::get<Index>(normal_args) = std::get<Index>(args);
  }
};

template<typename Params, typename Args, size_t Index>
struct normal_args_tuple_copier {
  using is_normal = gpu_kernel_dispatch::is_normal_arg< typename std::tuple_element<Index-1, Params>::type >;

  normal_args_tuple_copier(Args& args, typename normal_args_tuple_type<Params>::type& normal_args) {
    normal_args_tuple_single_copier<Params, Args, Index-1, is_normal::value>(args, normal_args);
    normal_args_tuple_copier<Params, Args, Index-1>(args, normal_args);
  }
};

template<typename Params, typename Args>
struct normal_args_tuple_copier<Params, Args, 0> {
  normal_args_tuple_copier(Args&, typename normal_args_tuple_type<Params>::type&) {
  }
};

template<typename Params, typename Args>
struct normal_args_container {
  using type = typename normal_args_tuple_type<Params>::type;
  type _tp;

  explicit normal_args_container(Args& args) :
    _tp()
  {
    normal_args_tuple_copier<Params, Args, std::tuple_size<Params>::value>(args, _tp);
  }
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
struct gpu_launch_info {
  bool dummy;
#ifdef SYMPHONY_HAVE_OPENCL
  clevent _cl_completion_event;
  cldevice* _d_ptr;
#endif
#ifdef SYMPHONY_HAVE_GLES
  GLsync _gl_fence;
#endif

  explicit gpu_launch_info(::symphony::internal::legacy::device_ptr const& device) :
    dummy(true)
#ifdef SYMPHONY_HAVE_OPENCL
    , _cl_completion_event()
    , _d_ptr( internal::c_ptr(device) )
#endif
#ifdef SYMPHONY_HAVE_GLES
    , _gl_fence()
#endif
  {
    SYMPHONY_UNUSED(device);
  }
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<size_t Dims, typename Fn, typename Params, typename Args>
class gputask : public gputask_base {
  static constexpr size_t num_buffer_args = num_buffer_ptrs_in_tuple<Args>::value;

private:
  gpu_launch_info _gli;
  Args _kernel_args;
  typename kernel_ptr_for_tupled_params<Params>::type _kernel_ptr;
  normal_args_container<Params, Args> _normal_args_container;
  ::symphony::range<Dims> _global_range;
  ::symphony::range<Dims> _local_range;
  ::symphony::internal::legacy::device_ptr const& _device;
  Fn& _fn;
  buffer_acquire_set< num_buffer_args > _buffer_acquire_set;
  preacquired_arenas<true, num_buffer_args> _preacquired_arenas;

  SYMPHONY_DELETE_METHOD(gputask(gputask const&));
  SYMPHONY_DELETE_METHOD(gputask(gputask&&));
  SYMPHONY_DELETE_METHOD(gputask& operator=(gputask const&));
  SYMPHONY_DELETE_METHOD(gputask& operator=(gputask&&));

  virtual executor_device get_executor_device() const;
  virtual bool do_execute(task_bundle_dispatch* tbd = nullptr);

public:
  template <typename Kernel>
  gputask(::symphony::internal::legacy::device_ptr const& device,
          const ::symphony::range<Dims>& r,
          const ::symphony::range<Dims>& l,
          Fn& f,
          legacy::task_attrs a,
          Kernel kernel,
          Args& kernel_args) :
    gputask_base(a)
    , _gli(device)
    , _kernel_args(kernel_args)
    , _kernel_ptr(kernel)
    , _normal_args_container(kernel_args)
    , _global_range(r)
    , _local_range(l)
    , _device(device)
    , _fn(f)
    , _buffer_acquire_set()
    , _preacquired_arenas()
  {}

  void unsafe_enable_non_locking_buffer_acquire() {
    _buffer_acquire_set.enable_non_locking_buffer_acquire();
  }

  void unsafe_register_preacquired_arena(bufferstate* bufstate,
                                         arena* preacquired_arena)
  {
    _preacquired_arenas.register_preacquired_arena(bufstate, preacquired_arena);
  }

  virtual void update_buffer_acquire_set(dyn_sized_bas& bas);

  virtual void release_arguments();
};

template<size_t Dims, typename Fn, typename Params, typename Args>
void
gputask<Dims, Fn, Params, Args>::update_buffer_acquire_set(dyn_sized_bas& bas)
{
  auto k_ptr = internal::c_ptr(_kernel_ptr);
  SYMPHONY_INTERNAL_ASSERT((k_ptr != nullptr), "null kernel");

#ifdef SYMPHONY_HAVE_OPENCL
  clkernel* kernel = nullptr;
#else
  glkernel* kernel = nullptr;
#endif

  gpu_kernel_dispatch::prepare_args_pass<Params, Args>
                                        (_device,
                                         kernel,
                                         std::forward<Args>(_kernel_args),
                                         _normal_args_container._tp,
                                         bas,
                                         true);
}

template<size_t Dims, typename Fn, typename Params, typename Args>
executor_device
gputask<Dims, Fn, Params, Args>::get_executor_device() const
{
  auto k_ptr = internal::c_ptr(_kernel_ptr);
  SYMPHONY_INTERNAL_ASSERT((k_ptr != nullptr), "null kernel");
#ifdef SYMPHONY_HAVE_OPENCL
  if(k_ptr->is_cl())
    return executor_device::gpucl;
#endif
#ifdef SYMPHONY_HAVE_GLES
  if(k_ptr->is_gl())
    return executor_device::gpugl;
#endif

  SYMPHONY_UNREACHABLE("Invalid kernel");
  return executor_device::unspecified;
}

class executor_construct {
public:
  explicit executor_construct(gputask_base* requestor_task,
                              task_bundle_dispatch* tbd = nullptr) :
    _requestor(requestor_task),
    _is_blocking(false),
    _is_bundled(tbd != nullptr),
    _is_last_in_bundle(tbd != nullptr && tbd->get_tasks().back() == requestor_task),
    _tbd(tbd)
  {
    SYMPHONY_INTERNAL_ASSERT(requestor_task != nullptr, "Invalid task");
    SYMPHONY_INTERNAL_ASSERT(is_bundled() || !is_last_in_bundle(),
                             "is_last_in_bundle cannot be true when not bundled");
  }

  executor_construct(void* blocking_requestor_id,
                     bool requestor_is_bundled,
                     bool requestor_is_last_in_bundle) :
    _requestor(blocking_requestor_id),
    _is_blocking(true),
    _is_bundled(requestor_is_bundled),
    _is_last_in_bundle(requestor_is_last_in_bundle),
    _tbd(nullptr)
  {
    SYMPHONY_INTERNAL_ASSERT(blocking_requestor_id != nullptr, "Invalid blocking requestor id -- needs to be unique");
    SYMPHONY_INTERNAL_ASSERT(is_bundled() || !is_last_in_bundle(),
                             "is_last_in_bundle cannot be true when not bundled");
  }

  void* get_requestor() const { return _requestor; }

  bool is_blocking() const { return _is_blocking; }

  bool is_bundled() const { return _is_bundled; }

  bool is_last_in_bundle() const { return _is_last_in_bundle; }

  task_bundle_dispatch* get_task_bundle() const {
    SYMPHONY_INTERNAL_ASSERT(!is_blocking(), "Construct is not a task");
    return _tbd;
  }

  gputask_base* get_requestor_task() const {
    SYMPHONY_INTERNAL_ASSERT(!is_blocking(), "Not a task");
    return static_cast<gputask_base*>(_requestor);
  }

  std::string to_string() const {
    return symphony::internal::strprintf("(requestor=%p %s %s %s tbd=%p)",
                                         _requestor,
                                         (_is_blocking ? "Blocking" : "NonBlocking"),
                                         (_is_bundled ? "Bundled" : "UnBundled"),
                                         (_is_last_in_bundle ? "Last" : "NotLast"),
                                         _tbd);
  }

private:
  void* _requestor;
  bool  _is_blocking;
  bool  _is_bundled;
  bool  _is_last_in_bundle;

  task_bundle_dispatch* _tbd;
};

#ifdef SYMPHONY_HAVE_OPENCL
SYMPHONY_GCC_IGNORE_BEGIN("-Wshadow");
template<typename Range>
void launch_cl_kernel(cldevice* d_ptr,
                      executor_construct const& exec_cons,
                      clkernel* cl_kernel,
                      Range& global_range,
                      Range& local_range,
                      clevent& cl_completion_event)
{
  SYMPHONY_INTERNAL_ASSERT(cl_kernel != nullptr, "Invalid cl_kernel");

  cl_completion_event = d_ptr->launch_kernel(cl_kernel, global_range, local_range);

  if(exec_cons.is_blocking())
    return;

  SYMPHONY_INTERNAL_ASSERT(!exec_cons.is_blocking(),
                           "Must be a task to be launched async");
  auto gputask = exec_cons.get_requestor();
  SYMPHONY_INTERNAL_ASSERT(gputask != nullptr,
                           "non-blocking execution requires valid gputask");

  auto tbd = exec_cons.get_task_bundle();
  SYMPHONY_INTERNAL_ASSERT(tbd != nullptr || !exec_cons.is_bundled(),
                           "Invalid bundle for a bundled task");

  if(!exec_cons.is_bundled() || exec_cons.is_last_in_bundle()) {
    auto cc       = exec_cons.is_bundled() ? &task_bundle_completion_callback : &completion_callback;
    auto userdata = exec_cons.is_bundled() ? static_cast<void*>(tbd) : gputask;

    cl_completion_event.get_impl().setCallback(CL_COMPLETE,
                                               cc,
                                               userdata);
  }
}
SYMPHONY_GCC_IGNORE_END("-Wshadow");
#endif

#ifdef SYMPHONY_HAVE_GLES
template<typename GLKernel, typename Range, typename GLFence>
void launch_gl_kernel(executor_construct const& exec_cons,
                      GLKernel* gl_kernel,
                      Range& global_range,
                      Range& local_range,
                      GLFence& gl_fence)
{
  SYMPHONY_INTERNAL_ASSERT(gl_kernel != nullptr, "Invalid gl_kernel");
  SYMPHONY_API_ASSERT(!exec_cons.is_bundled(), "GL dispatch does not as yet support bundle dispatch");

  gl_fence = gl_kernel->launch(global_range, local_range);

  GLenum error = glClientWaitSync(gl_fence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
  if(GL_TIMEOUT_EXPIRED == error) {
    SYMPHONY_DLOG("GL_TIMEOUT_EXPIRED");
  }
  else if((GL_ALREADY_SIGNALED == error) || (GL_CONDITION_SATISFIED == error)) {
    if(exec_cons.is_blocking()) {

    }
    else {
      SYMPHONY_DLOG("--- GL stub task ---");
      auto gputask = exec_cons.get_requestor_task();
      gputask->on_finish();
      gputask->finish(false, nullptr, true, true);
    }
  }
  else if(GL_WAIT_FAILED == error) {
    SYMPHONY_DLOG("GL_WAIT_FAILED");
  }
}
#endif

template<size_t Dims,
         typename Params,
         typename Args,
         typename KernelPtr,
         typename NormalArgs,
         typename BAS,
         typename PreacquiredArenas>
bool gpu_do_execute(KernelPtr k_ptr,
                    executor_construct const& exec_cons,
                    gpu_launch_info& gli,
                    ::symphony::internal::legacy::device_ptr const& device,
                    ::symphony::range<Dims> const& global_range,
                    ::symphony::range<Dims> const& local_range,
                    Args&& kernel_args,
                    NormalArgs& normal_args,
                    BAS& bas,
                    PreacquiredArenas const* p_preacquired_arenas,
                    bool add_buffers,
                    bool perform_launch)
{
  SYMPHONY_INTERNAL_ASSERT(exec_cons.get_requestor() != nullptr,
                           "Invalid requestor in executor construct");

#ifdef SYMPHONY_HAVE_OPENCL

#ifdef SYMPHONY_HAVE_GLES
  char const* kernel_type = (k_ptr->is_cl() ? "OpenCL" : k_ptr->is_gl() ? "OpenGLES" : "INVALID");
#else
  char const* kernel_type = "OpenCL";
#endif

#else
  char const* kernel_type = "OpenGLES";
#endif
  SYMPHONY_UNUSED(kernel_type);

  char last_in_bundle = exec_cons.is_last_in_bundle() ? 'Y' : 'N';
  SYMPHONY_UNUSED(last_in_bundle);

  if(exec_cons.is_blocking()) {
    SYMPHONY_DLOG("executing %s executor id %p bundled=%c last_in_bundle=%c",
                  kernel_type,
                  exec_cons.get_requestor(),
                  exec_cons.is_bundled() ? 'Y' : 'N',
                  last_in_bundle);
  }
  else {
    if(exec_cons.is_bundled())
      SYMPHONY_DLOG("enqueuing %s task %p for bundle %p last_in_bundle=%c",
                    kernel_type,
                    exec_cons.get_requestor_task(),
                    exec_cons.get_task_bundle(),
                    last_in_bundle);
    else
      SYMPHONY_DLOG("enqueuing %s task %p", kernel_type, exec_cons.get_requestor_task());
  }

#ifdef SYMPHONY_HAVE_OPENCL
  SYMPHONY_INTERNAL_ASSERT((gli._d_ptr != nullptr), "null device");
  if(k_ptr->is_cl())
    SYMPHONY_DLOG("cl kernel: %p", k_ptr->get_cl_kernel());
#endif
#ifdef SYMPHONY_HAVE_GLES
  if(k_ptr->is_gl())
    SYMPHONY_DLOG("gl kernel: %p", k_ptr->get_gl_kernel());
#endif

  std::unique_lock<std::mutex> ul(k_ptr->access_dispatch_mutex(), std::defer_lock);
  if(perform_launch)
    ul.lock();

  void const* requestor = exec_cons.get_requestor();
  bool conflict = false;
  if(!exec_cons.is_bundled()) {
    conflict =
        !gpu_kernel_dispatch::prepare_args<Params, Args>(
                device,
                k_ptr,
                std::forward<Args>(kernel_args),
                normal_args,
                bas,
                p_preacquired_arenas,
                requestor,
                add_buffers,
                perform_launch,
                perform_launch);
  }
  else if(exec_cons.is_blocking()) {
    preacquired_arenas_base* paa = nullptr;
    conflict =
        !gpu_kernel_dispatch::prepare_args<Params, Args>(
                device,
                k_ptr,
                std::forward<Args>(kernel_args),
                normal_args,
                bas,
                paa,
                requestor,
                add_buffers,
                false,
                perform_launch);
    SYMPHONY_INTERNAL_ASSERT(!conflict,
                             "No conflict should be found as no buffer acquires would be attempted.");
  }
  else {
    SYMPHONY_INTERNAL_ASSERT(!add_buffers,
                             "Re-attemping add buffers: Task bundle dispatch would have already added buffers.");
    SYMPHONY_INTERNAL_ASSERT(exec_cons.get_task_bundle()->get_buffer_acquire_set().acquired(),
                             "Task bundle dispatch should have already acquired buffers.");
    auto tbd = exec_cons.get_task_bundle();
    conflict =
        !gpu_kernel_dispatch::prepare_args<Params, Args>(
                device,
                k_ptr,
                std::forward<Args>(kernel_args),
                normal_args,
                tbd->get_buffer_acquire_set(),
                &(tbd->get_preacquired_arenas()),
                requestor,
                false,
                false,
                perform_launch);
    SYMPHONY_INTERNAL_ASSERT(!conflict,
                             "No conflict should be found as no buffer acquires would be attempted.");
  }

  if(conflict)
    return false;

  if(!perform_launch)
    return true;

  if(0) {}
#ifdef SYMPHONY_HAVE_OPENCL
  else if(k_ptr->is_cl()) {
    launch_cl_kernel(gli._d_ptr,
                     exec_cons,
                     k_ptr->get_cl_kernel(),
                     global_range,
                     local_range,
                     gli._cl_completion_event);

    if(exec_cons.is_blocking()) {
      if(!exec_cons.is_bundled() || exec_cons.is_last_in_bundle()) {
        bas.release_buffers(requestor);
        gli._d_ptr->get_cmd_queue().finish();
      }
    }
  }
#endif
#ifdef SYMPHONY_HAVE_GLES
  else if(k_ptr->is_gl()) {
    launch_gl_kernel(exec_cons,
                     k_ptr->get_gl_kernel(),
                     global_range,
                     local_range,
                     gli._gl_fence);
  }
#endif
  else
    SYMPHONY_FATAL("Unsupported type of GPU kernel");

  return true;
}

template<size_t Dims, typename Fn, typename Params, typename Args>
bool
gputask<Dims, Fn, Params, Args>::do_execute(task_bundle_dispatch* tbd)
{
  auto k_ptr = internal::c_ptr(_kernel_ptr);

  SYMPHONY_INTERNAL_ASSERT((k_ptr != nullptr), "null kernel");

  SYMPHONY_INTERNAL_ASSERT(get_snapshot().is_running(),
                       "Can't execute task %p: %s",
                       this, to_string().c_str());

  auto is_bundle_dispatch = tbd != nullptr;
  bool launched = gpu_do_execute<Dims, Params, Args>
                                (k_ptr,
                                 executor_construct(this, tbd),
                                 _gli,
                                 _device,
                                 _global_range,
                                 _local_range,
                                 std::forward<Args>(_kernel_args),
                                 _normal_args_container._tp,
                                 _buffer_acquire_set,
                                 &_preacquired_arenas,
                                 !is_bundle_dispatch && _first_execution,
                                 true);
  _first_execution = false;

  if(!launched)
    return false;

  auto guard = get_lock_guard();
  _executed = true;

  return true;
}

template<size_t Dims, typename Fn, typename Params, typename Args>
void
gputask<Dims, Fn, Params, Args>::release_arguments()
{
    SYMPHONY_DLOG("release_arguments");
    _buffer_acquire_set.release_buffers(this);
}

};
};

#endif
