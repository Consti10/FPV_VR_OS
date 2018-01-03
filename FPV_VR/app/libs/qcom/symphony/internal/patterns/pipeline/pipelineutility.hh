// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once
#include <symphony/internal/buffer/bufferpolicy.hh>

namespace symphony{

namespace pattern{
  class tuner;
}

namespace internal{
namespace pipeline_utility{

enum stage_param_list_type {
  symphony_iteration_lag,
  symphony_iteration_rate,
  symphony_sliding_window_size,
  symphony_stage_pattern_tuner
};

enum launch_param_list_type {
  symphony_num_iterations,
  symphony_launch_pattern_tuner
};

static inline size_t get_gcd(size_t a, size_t b) {
  while (a * b != 0)  {
    if (a >= b) a = a % b;
    else b = b % a;
  }
  return (a + b);
}

static inline size_t get_succ_iter(size_t rate_curr, size_t rate_succ, size_t curr_iter) {
  return curr_iter * rate_succ / rate_curr;
}

static inline size_t get_pred_iter(size_t rate_pred, size_t rate_curr, size_t curr_iter) {
  return (curr_iter * rate_pred + rate_curr - 1) / rate_curr;
}

template<typename T, size_t arity>
struct get_body_arg1_type  {
  using type = void;
};

template<typename T>
struct get_body_arg1_type<T, 2> {
  using type = typename internal::function_traits<T>::template arg_type<1>;
};

template<size_t arity>
struct get_body_arg1_type<void, arity> {
  using type = void;
};

template<typename T, size_t arity, typename B>
struct is_body_arg1_based_on_b;

template<typename T, typename B>
struct is_body_arg1_based_on_b<T, 2, B> {
  using arg1_type = typename internal::function_traits<T>::template arg_type<1>;
  using arg1_type_noref = typename std::remove_reference<arg1_type>::type;

  static_assert(std::is_reference<arg1_type>::value,
    "the second argument of a cpu stage lambda/function shuold always be a reference to symphony::stage_input<T>.");

  static constexpr bool value =
   std::is_base_of<B, arg1_type_noref>::value ? true : false;
};

template<typename T, typename B>
struct is_body_arg1_based_on_b<T, 1, B> {
  static constexpr bool value = true;
};

template<size_t arity, typename B>
struct is_body_arg1_based_on_b<void, arity, B> {
  static constexpr bool value = true;
};

template <typename... Args>
struct is_std_tuple {
  static constexpr bool value = false;
};

template <typename... Args>
struct is_std_tuple<std::tuple<Args...>> {
  static constexpr bool value = true;
};

template<typename T>
struct strip_stage_input_type  {
  using type = T;
};

template<typename T>
struct strip_stage_input_type<symphony::stage_input<T>&> {
  using type = T;
};

template<typename T>
struct is_parallel_stage  {
  static constexpr bool value = false;
};

template<>
struct is_parallel_stage<symphony::parallel_stage> {
  static constexpr bool value = true;
};

template<typename T>
struct is_serial_stage {
  static constexpr bool value = false;
};

template<>
struct is_serial_stage<symphony::serial_stage> {
  static constexpr bool value = true;
};

template<typename T>
struct is_iteration_lag {
  static constexpr bool value = false;
};

template<>
struct is_iteration_lag<symphony::iteration_lag> {
  static constexpr bool value = true;
};

template<typename T>
struct is_iteration_rate {
  static constexpr bool value = false;
};

template<>
struct is_iteration_rate<symphony::iteration_rate> {
  static constexpr bool value = true;
};

template<typename T>
struct is_sliding_window_size {
  static constexpr bool value = false;
};

template<>
struct is_sliding_window_size<symphony::sliding_window_size> {
  static constexpr bool value = true;
};

template<typename T>
struct is_pattern_tuner {
  static constexpr bool value = false;
};

template<>
struct is_pattern_tuner<symphony::pattern::tuner> {
  static constexpr bool value = true;
};

template<typename T>
struct is_symphony_pipeline_callable {
private:
  template <typename U >
  static auto check(int)-> decltype(&U::operator(), std::true_type());

  template <typename>
  static std::false_type check(...);

  using enable_if_type = decltype(check<T>(0));

public:
  using type = typename enable_if_type::type;
  enum{
    value = type::value
  };
};

template <typename FReturnType, typename... FArgs>
struct is_symphony_pipeline_callable<FReturnType(&)(FArgs...)> {
  static constexpr bool value = true;
};

template <typename FReturnType, typename... FArgs>
struct is_symphony_pipeline_callable<FReturnType(*)(FArgs...)> {
  static constexpr bool value = true;
};

template<int index, typename TP>
struct tuple_element_type_helper  {
  using type = typename std::tuple_element<index, TP>::type;
};

template<typename TP>
struct tuple_element_type_helper<-1, TP> {
  using type = void;
};

template<int index, typename TP>
struct stage_type_parser {
  using prior = stage_type_parser<index - 1, TP>;
  using type  = typename std::remove_cv<
                  typename std::remove_reference<
                    typename std::tuple_element<index, TP>::type>::type>::type;

  static_assert(
    prior::pindex == -1 || !is_parallel_stage<type>::value,
    "Multiple parallel_stage definitions found in the pipeline");

  static_assert(
    prior::sindex == -1 || !is_serial_stage<type>::value,
    "Multiple serial_stage definitions found in the pipeline");

  static constexpr int pindex =
   (prior::pindex != -1 ? prior::pindex : is_parallel_stage<type>::value ? index : -1);

  static constexpr int sindex =
   (prior::sindex != -1 ? prior::sindex : is_serial_stage<type>::value ? index : -1);
};

template<typename TP>
struct stage_type_parser<-1, TP>  {
  static constexpr int pindex = -1;
  static constexpr int sindex = -1;
};

template<typename... Args>
struct check_stage_type {
  using TP = std::tuple<Args...>;
  using result = stage_type_parser<std::tuple_size<TP>::value - 1, TP>;

  static_assert(result::pindex != -1 || result::sindex != -1,
    "a pipeline stage should be specified to be either serial or parallel.");

  static_assert(result::pindex == -1 || result::sindex == -1,
    "a pipeline stage cannot be serial and parallel at the same time.");

  static constexpr bool has_parallel_stage = result::pindex == -1 ? false : true;
  static constexpr int pindex = result::pindex;
  static constexpr int sindex = result::sindex;
  static constexpr int index = has_parallel_stage ? pindex : sindex;

  using type = typename std::conditional<has_parallel_stage,
                                         symphony::parallel_stage,
                                         symphony::serial_stage>::type;
};

template<int index, typename TP, int offset>
struct callable_parser  {
  using prior = callable_parser<index - 1, TP, offset - 1>;
  using type  = typename std::tuple_element<index, TP>::type;
  using unref_type  =
    typename std::remove_reference<
      typename std::tuple_element<index, TP>::type>::type;

  static constexpr bool is_callable =
    is_symphony_pipeline_callable<type>::value || is_symphony_pipeline_callable<unref_type>::value;

  static_assert(prior::callable_index == -1 || !is_callable,
    "Multiple lambdas found for the pipeline cpu stage");

  static constexpr int callable_index =
   (prior::callable_index != -1 ?
    prior::callable_index : is_callable ? index : -1);
};

template<int index, typename TP>
struct callable_parser<index, TP, -1>  {
  static constexpr int callable_index = -1;
};

template<typename... Args>
struct extract_callable_lambda {
  using TP = std::tuple<Args...>;
  using result =
    callable_parser<std::tuple_size<TP>::value - 1,
                    TP,
                    std::tuple_size<TP>::value - 1>;

  static constexpr int index = result::callable_index;
  using type = typename tuple_element_type_helper<index, TP>::type;
};

template<int index, typename TP>
struct parse_add_stage_params_appearance {
  using prior = parse_add_stage_params_appearance<index - 1, TP >;
  using type  = typename std::remove_cv<
                  typename std::remove_reference<
                    typename std::tuple_element<index, TP>::type>::type>::type;

  static constexpr size_t parallel_stage_num =
    is_parallel_stage<type>::value ? prior::parallel_stage_num + 1 : prior::parallel_stage_num;
  static_assert(parallel_stage_num < 2, "Multiple parallel_stage defined when adding the pipeline stage");

  static constexpr size_t serial_stage_num =
    is_serial_stage<type>::value ? prior::serial_stage_num + 1 : prior::serial_stage_num;
  static_assert(serial_stage_num < 2, "Multiple serial_stage defined when adding the pipeline stage");

  static constexpr size_t iteration_lag_num =
    is_iteration_lag<type>::value ? prior::iteration_lag_num + 1 : prior::iteration_lag_num;
  static_assert(iteration_lag_num < 2, "Multiple iteration_lag defined when adding the pipeline stage");

  static constexpr size_t iteration_rate_num =
    is_iteration_rate<type>::value ? prior::iteration_rate_num + 1 : prior::iteration_rate_num;
  static_assert(iteration_rate_num < 2, "Multiple iteration_rate defined when adding the pipeline stage");

  static constexpr size_t sliding_window_size_num =
    is_sliding_window_size<type>::value ? prior::sliding_window_size_num + 1 : prior::sliding_window_size_num;
  static_assert(sliding_window_size_num < 2, "Multiple sliding_window_size defined when adding the pipeline stage");

  static constexpr size_t pattern_tuner_num =
    is_pattern_tuner<type>::value ? prior::pattern_tuner_num + 1 : prior::pattern_tuner_num;
  static_assert(pattern_tuner_num < 2, "Multiple pattern tuners defined when adding the pipeline stage");

  static constexpr int iteration_lag_index =
    is_iteration_lag<type>::value ? index : prior::iteration_lag_index;

  static constexpr int iteration_rate_index =
    is_iteration_rate<type>::value ? index : prior::iteration_rate_index;

  static constexpr int sliding_window_size_index =
    is_sliding_window_size<type>::value ? index : prior::sliding_window_size_index;

  static constexpr int pattern_tuner_index =
    is_pattern_tuner<type>::value ? index : prior::pattern_tuner_index;
};

template<typename TP>
struct parse_add_stage_params_appearance<-1, TP>  {
  static constexpr size_t parallel_stage_num        = 0;
  static constexpr size_t serial_stage_num          = 0;
  static constexpr size_t iteration_lag_num         = 0;
  static constexpr size_t iteration_rate_num        = 0;
  static constexpr size_t sliding_window_size_num   = 0;
  static constexpr size_t pattern_tuner_num         = 0;

  static constexpr int iteration_lag_index          = -1;
  static constexpr int iteration_rate_index         = -1;
  static constexpr int sliding_window_size_index    = -1;
  static constexpr int pattern_tuner_index          = -1;
};

template<typename ContextRefType, typename... Args>
struct check_add_cpu_stage_params {
  static constexpr bool value = true;
  using TP = std::tuple<Args...>;
  using result =
    parse_add_stage_params_appearance<std::tuple_size<TP>::value - 1, TP>;

  using Body                       = typename extract_callable_lambda<Args...>::type;
  static constexpr int body_index  = extract_callable_lambda<Args...>::index;
  static_assert(body_index != -1, "a cpu stage should have its function defined");

  using StageType                  = typename check_stage_type<Args...>::type;
  static constexpr int stage_index = check_stage_type<Args...>::index;
  static_assert(stage_index != -1, "a cpu stage should specify its type, i.e. serial or parallel");

  using arg0_type = typename function_traits<Body>::template arg_type<0>;
  static_assert(
    std::is_same<arg0_type, ContextRefType>::value,
      "The 1st param for a stage function should be the pipeline context ref.");
  static_assert(
                is_body_arg1_based_on_b<Body, function_traits<Body>::arity::value, symphony::stage_input_base>::value,
    "The 2nd param for a cpu stage function should be a reference to type stage_input<xxx>");

  static_assert(result::parallel_stage_num == 1 || result::serial_stage_num == 1,
    "a cpu stage should specify its type, i.e. serial or parallel");
};

template<int index, typename TP>
struct parse_launch_params_appearance {
  using prior = parse_launch_params_appearance<index - 1, TP >;
  using type  = typename std::remove_cv<
                  typename std::remove_reference<
                    typename std::tuple_element<index, TP>::type>::type>::type;
  static constexpr size_t pattern_tuner_num =
    is_pattern_tuner<type>::value ? prior::pattern_tuner_num + 1 : prior::pattern_tuner_num;
  static_assert(pattern_tuner_num < 2, "Multiple pattern tuners defined when adding the pipeline stage");

  static constexpr int pattern_tuner_index =
    is_pattern_tuner<type>::value ? index : prior::pattern_tuner_index;
};

template<typename TP>
struct parse_launch_params_appearance<-1, TP>  {
  static constexpr size_t pattern_tuner_num         = 0;

  static constexpr int pattern_tuner_index          = -1;
};

template<typename... Args>
struct check_launch_params {
  static constexpr bool value = true;
  using TP = std::tuple<Args...>;
  using result =
    parse_launch_params_appearance<std::tuple_size<TP>::value - 1, TP>;
};

template <typename T>
struct type_helper {
  using org_type =
    typename std::conditional<std::is_same<T, void*>::value,
                              void*,
                              typename std::remove_reference<T>::type>::type;

  using ref_type =
    typename std::conditional<std::is_same<T, void*>::value,
                              void*,
                              org_type&>::type;

  using rref_type =
    typename std::conditional<std::is_same<T, void*>::value,
                              void*,
                              org_type&&>::type;
};

template <typename T>
struct get_ringbuffer_type  {
  using type =
    typename std::conditional<std::is_same<T, void>::value,
                              symphony::internal::stagebuffer,
                              symphony::internal::ringbuffer<T>>::type;
};

template <typename T>
struct get_dynamicbuffer_type  {
  using type =
    typename std::conditional<std::is_same<T, void>::value,
                              symphony::internal::stagebuffer,
                              symphony::internal::dynamicbuffer<T>>::type;
};

template<int index, typename TP>
struct get_tuple_element_helper {
  static typename std::tuple_element<index, TP>::type
  get(TP& t)  {
    using elem_type = typename std::tuple_element<index, TP>::type;
    return std::forward<elem_type>(std::get<index>(t));
  }
};

template<typename TP>
struct get_tuple_element_helper<-1, TP>  {
  static int* get(TP& ) {
    return nullptr;
  }
};

template <typename RT, typename TP1, typename TP2, int index1, int index2>
struct mux_param_value {
  static RT get(TP1& tp1, TP2&) {
    using elem_type =
      typename std::remove_cv<
        typename std::remove_reference<
         typename std::tuple_element<index1, TP1>::type>::type>::type;
    static_assert(std::is_same<elem_type, RT>::value, "return type does not match");
    return std::get<index1>(tp1);
  }
};

template <typename RT, typename TP1, typename TP2, int index2>
struct mux_param_value <RT, TP1, TP2, -1, index2> {
  static RT get(TP1&, TP2& tp2) {
    using elem_type =
      typename std::remove_cv<
       typename std::remove_reference<
         typename std::tuple_element<index2, TP2>::type>::type>::type;
    static_assert(std::is_same<elem_type, RT>::value, "return type does not match");
    return std::get<index2>(tp2);
  }
};

template<typename T, int id>
struct get_cpu_body_type  {
  static_assert(id != -1, "a CPU stage should always have one body,"
    "i.e. lambda/callable object /function ptr");
  using type = typename::std::tuple_element<id, T>::type;
};

template<size_t N>
struct apply_launch {
  template<typename CPINST, typename LAUNCHTYPE, typename TUNER, typename TUPLE, typename... T>
  static void apply(CPINST* cpinst, LAUNCHTYPE& launch_type, TUNER const& tuner, TUPLE& tp, T&... t) {
    apply_launch<N-1>::apply(cpinst, launch_type, tuner, tp, std::get<N-1>(tp), t...);
  }
};

template<>
struct apply_launch<0> {
  template<typename CPINST, typename LAUNCHTYPE, typename TUNER, typename TUPLE, typename... T>
  static void apply(CPINST* cpinst, LAUNCHTYPE& launch_type, TUNER const& tuner, TUPLE&, T&... t) {
    cpinst->launch(t..., launch_type, tuner);
  }
};

template<typename... Args>
struct is_gpu_kernel {
  static constexpr bool value = false;
};

template<typename... Args>
struct is_gpu_kernel<symphony::gpu_kernel<Args...>&> {
  static constexpr bool value = true;
};

template<typename... Args>
struct is_gpu_kernel<symphony::gpu_kernel<Args...>> {
  static constexpr bool value = true;
};

template<int index, typename TP>
struct gpu_kernel_parser {
  using prior = gpu_kernel_parser<index - 1, TP>;
  using type  = typename std::tuple_element<index, TP>::type;

  static_assert(
    prior::gk_index == -1 || !is_gpu_kernel<type>::value,
    "Multiple gpu kernel found for the pipeline gpu stage");

  static constexpr int gk_index =
   (prior::gk_index != -1 ? prior::gk_index : is_gpu_kernel<type>::value ? index : -1);
};

template<typename TP>
struct gpu_kernel_parser<-1, TP>  {
  static constexpr int gk_index = -1;
};

template<typename... Args>
struct check_gpu_kernel {
  using TP = std::tuple<Args...>;
  using result = gpu_kernel_parser<std::tuple_size<TP>::value - 1, TP>;

  static constexpr bool has_gpu_kernel = result::gk_index == -1 ? false : true;
  static constexpr int index = result::gk_index;

  using type = typename tuple_element_type_helper<index, TP>::type;
};

#ifndef SYMPHONY_HAVE_RTTI
template <typename T>
struct sizeof_type {
  static constexpr size_t size = sizeof(T);
};

template <>
struct sizeof_type<void> {
  static constexpr size_t size = 0;
};
#endif

#ifdef SYMPHONY_HAVE_GPU

template <typename T>
struct get_return_tuple_type  {
  using type =
    typename std::conditional<
      std::is_same<T, void>::value,
      std::tuple<>,
      T>::type;
};

template<typename ...Args>
struct is_symphony_pipeline_callable<symphony::gpu_kernel<Args...>&> {
  static constexpr bool value = false;
};

template<typename ...Args>
struct is_symphony_pipeline_callable<symphony::gpu_kernel<Args...>> {
  static constexpr bool value = false;
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template <typename T>
struct is_symphony_range : std::false_type {
  static constexpr bool value = false;
};

template <size_t Dims>
struct is_symphony_range<symphony::range<Dims>> : std::true_type {
  static constexpr bool value = true;
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename Arg, typename... Args>
struct remove_first_tuple_element_type;

template<typename Arg, typename... Args>
struct remove_first_tuple_element_type<std::tuple<Arg, Args...>> {
  using type = std::tuple<Args...>;
};

template<typename Arg, typename... Args>
struct remove_first_tuple_element_type<std::tuple<Arg, Args...>&> {
  using type = std::tuple<Args...>&;
};

template <typename... BufDirTypes>
struct strip_buffer_dir_params;

template <typename... BufDirTypes>
struct strip_buffer_dir_params<std::tuple<BufDirTypes...>> {
  using stripped_tuple_type = std::tuple<typename strip_buffer_dir<BufDirTypes>::type...>;
};

template<typename T, size_t arity>
struct get_af_arg1_type  {
  using type = void;
};

template<typename T>
struct get_af_arg1_type<T, 2> {
  using type = typename internal::function_traits<T>::template arg_type<1>;
};

template<size_t arity>
struct get_af_arg1_type<void, arity> {
  using type = void;
};

template<typename T, int id>
struct get_body_type  {
  using type = typename::std::tuple_element<id, T>::type;
};

template<typename T>
struct get_body_type<T, -1>  {
  using type = void;
};

template<typename... Args>
struct extract_before_lambda {
  using TP = std::tuple<Args...>;

  static_assert(check_gpu_kernel<Args...>::has_gpu_kernel,
    "extracting before lambda from a non-gpu stage");

  using result =
    callable_parser<check_gpu_kernel<Args...>::index - 1,
                    TP,
                    check_gpu_kernel<Args...>::index - 1>;

  static constexpr int index = result::callable_index;

  using type = typename tuple_element_type_helper<index, TP>::type;
};

template<typename... Args>
struct extract_after_lambda {
  using TP = std::tuple<Args...>;

  static_assert(check_gpu_kernel<Args...>::has_gpu_kernel,
    "extracting after lambda from a non-gpu stage");

  using result =
    callable_parser<std::tuple_size<TP>::value - 1,
                    TP,
                    std::tuple_size<TP>::value - check_gpu_kernel<Args...>::index - 1>;

  static constexpr int index = result::callable_index;

  using type = typename tuple_element_type_helper<index, TP>::type;
};

template<typename... Args>
struct get_gpu_kernel_args_tuple;

template<typename... Args>
struct get_gpu_kernel_args_tuple<symphony::gpu_kernel<Args...>> {
  using type = typename std::tuple<Args...>;
};

template<typename... Args>
struct get_gpu_kernel_args_tuple<symphony::gpu_kernel<Args...>&> {
  using type = typename std::tuple<Args...>;
};

template<typename ContextRefType, typename InputTupleType, typename... Args>
struct check_add_gpu_stage_params {
  static constexpr bool value = true;
  using TP = std::tuple<Args...>;
  using result =
    parse_add_stage_params_appearance<std::tuple_size<TP>::value - 1, TP>;

  static constexpr int gkbody_index = check_gpu_kernel<Args...>::index;
  using GKBody            =
    typename get_body_type<InputTupleType, gkbody_index>::type;
  using GKParamsTupleType =
    typename get_gpu_kernel_args_tuple<GKBody>::type;
  static_assert(gkbody_index != -1, "gpu stage should have a gpu kernel");

  static constexpr int bbody_index  = extract_before_lambda<Args...>::index;
  using BeforeBody        =
    typename get_body_type<InputTupleType, bbody_index>::type;

  static constexpr int abody_index  = extract_after_lambda<Args...>::index;
  using AfterBodyVoid  = typename get_body_type<InputTupleType, abody_index>::type;

  using AfterBody =
    typename std::conditional<std::is_same<AfterBodyVoid, void>::value,
                              void*,
                              AfterBodyVoid>::type;

  static_assert(!std::is_same<BeforeBody, void>::value, "a gpu stage should have a before lambda");

  using before_arg0_type = typename internal::function_traits<BeforeBody>::template arg_type<0>;
  static_assert(
    std::is_same<before_arg0_type, ContextRefType>::value,
    "The 1st param for the before functor of a gpu stage should be the pipeline context ref.");

  using UniAfterBody =
     typename std::conditional<std::is_same<AfterBodyVoid, void>::value,
                               void(&)(void*),
                               AfterBody>::type;

  using after_arg0_type = typename internal::function_traits<UniAfterBody>::template arg_type<0>;

  static_assert(
    std::is_same<after_arg0_type, ContextRefType>::value || std::is_same<after_arg0_type, void*>::value,
    "The 1st param for the after functor of a gpu stage should be the pipeline context ref.");

  using before_return_type = typename internal::function_traits<BeforeBody>::return_type;
  static_assert(is_std_tuple<before_return_type>::value, "before lamdba should return a std::tuple");
  static_assert(
    std::tuple_size<before_return_type>::value >= 2,
    "before lambda's return tuple should have at least two elements, i.e. a symphony::range, and one input for the gpu kernel");

  using before_return_type_first_element = typename std::tuple_element<0, before_return_type>::type;
  static_assert(
    is_symphony_range<before_return_type_first_element>::value,
    "before lambda's return tuple should be a std::tuple whose first element is a symphony::range");

  using before_return_type_wo_first_element = typename remove_first_tuple_element_type<before_return_type>::type;
  using gk_params_tuple_type = typename strip_buffer_dir_params<GKParamsTupleType>::stripped_tuple_type;
  static_assert(
    std::is_same<before_return_type_wo_first_element, gk_params_tuple_type>::value,
    "The return type of the before functor of a gpu stage should be an std::tuple of the gpu kernel parameters.");

  using StageType                  = typename check_stage_type<Args...>::type;
  static constexpr int stage_index = check_stage_type<Args...>::index;
  static_assert(stage_index != -1, "a gpu stage should specify its type, i.e. serial or parallel");

  static_assert(result::parallel_stage_num == 1 || result::serial_stage_num == 1,
    "a gpu stage should specify its type, i.e. serial or parallel");
};
#endif

};
};
};
