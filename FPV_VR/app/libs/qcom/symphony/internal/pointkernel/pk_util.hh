// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include<string>

namespace symphony{

namespace internal{

namespace pointkernel{

template<typename ...L>
struct arg_list_gen {
  template <typename M, typename ...N>
    struct append {
      typedef arg_list_gen<M,N...,L...> type;
    };
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename T>
struct is_range : std::integral_constant<bool, false> {};

template<>
struct is_range<symphony::range<1>> : std::integral_constant<bool, true> {};

template<>
struct is_range<symphony::range<2>> : std::integral_constant<bool, true> {};

template<>
struct is_range<symphony::range<3>> : std::integral_constant<bool, true> {};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

struct cpu_arg_list {
typedef arg_list_gen<> type;
};

template<bool isRange, typename ...Args> struct cpu_expand_range;

template<typename ...Args>
struct cpu_expand_range<false, Args...> {
  typedef typename cpu_arg_list::type::template
    append<Args...>::type type;
};

template<typename ...Args>
struct cpu_expand_range<true, symphony::range<1>, Args...>{
  typedef typename cpu_arg_list::type::template
    append<symphony::range<1>, symphony::pattern::tuner, Args...>::type type;
};

template<typename ...Args>
struct cpu_expand_range<true, symphony::range<2>, Args...>{
  typedef typename cpu_arg_list::type::template
  append<symphony::range<2>, symphony::pattern::tuner, Args...>::type type;
};

template<typename ...Args>
struct cpu_expand_range<true, symphony::range<3>, Args...>{
  typedef typename cpu_arg_list::type::template
    append<symphony::range<3>, symphony::pattern::tuner, Args...>::type type;
};

template<typename F, typename ...Args>
struct make_cpu_kernel_args{
  typedef typename cpu_expand_range<is_range<F>::value, F, Args...>::type type;
};

template<typename... Args> struct cpu_kernel_generator;

template<typename ReturnType, typename... Args>
struct cpu_kernel_generator<ReturnType, arg_list_gen<Args...>>
{
  typedef symphony::cpu_kernel<ReturnType(Args...)> type;
};

template<typename... Args> struct cpu_kernel_signature_generator;

template<typename ReturnType, typename... Args>
struct cpu_kernel_signature_generator<ReturnType, arg_list_gen<Args...>>
{
  typedef ReturnType(*type)(Args...);
};

template<bool is_pointer, bool is_integer, typename ...Args> struct gpu_arg_list;

template <>
struct gpu_arg_list<false, false> {
typedef arg_list_gen<> type;
};

template<typename First>
struct gpu_arg_list<false, false, First> {
  typedef typename gpu_arg_list<false, false>::type::template
    append<First>::type type;
};

template<typename First, typename Second>
struct gpu_arg_list<true, true, First, Second> {
  typedef typename gpu_arg_list<false, false>::type::template
    append<typename symphony::buffer_ptr<typename std::remove_pointer<First>::type>, Second>::type type;
};

template<typename First, typename Second>
struct gpu_arg_list<true, false, First, Second> {
  typedef typename gpu_arg_list<false, false>::type::template
    append<First, Second>::type type;
};

template<typename First, typename Second>
struct gpu_arg_list<false, true, First, Second> {
  typedef typename gpu_arg_list<false, false>::type::template
    append<First, Second>::type type;
};

template<typename First, typename Second>
struct gpu_arg_list<false, false, First, Second> {
  typedef typename gpu_arg_list<false, false>::type::template
    append<First, Second>::type type;
};

template<typename First, typename Second, typename Third, typename ...Args>
struct gpu_arg_list<false,false, First, Second, Third, Args...> {
  typedef typename gpu_arg_list<std::is_pointer<Second>::value, std::is_integral<Third>::value,
                                Second, Third, Args...>::type::template append<First>::type type;
};

template<typename First, typename Second, typename Third, typename ...Args>
struct gpu_arg_list<true,false, First, Second, Third, Args...> {
  typedef typename gpu_arg_list<std::is_pointer<Second>::value, std::is_integral<Third>::value,
                                Second, Third, Args...>::type::template append<First>::type type;
};

template<typename First, typename Second, typename Third, typename ...Args>
struct gpu_arg_list<false,true, First, Second, Third, Args...> {
  typedef typename gpu_arg_list<std::is_pointer<Second>::value, std::is_integral<Third>::value,
                                Second, Third, Args...>::type::template append<First>::type type;
};

template<typename First, typename Second, typename Third, typename ...Args>
struct gpu_arg_list<true,true, First, Second, Third, Args...> {
  typedef typename gpu_arg_list<std::is_pointer<Second>::value, std::is_integral<Third>::value,
                                Second, Third, Args...>::type::template
      append<typename symphony::buffer_ptr<typename std::remove_pointer<First>::type>>::type type;
};

template<bool isRange, typename ...Args> struct gpu_discard_range;

template<typename Range>
struct gpu_discard_range<true, Range> {
  typedef typename gpu_arg_list<false, false>::type type;
};

template<typename Range, typename First>
struct gpu_discard_range<true, Range, First> {
  typedef typename gpu_arg_list<false, false, First>::type type;
};

template<typename First, typename Second, typename ...Args>
struct gpu_discard_range<false, First, Second, Args...> {
  typedef typename gpu_arg_list<std::is_pointer<First>::value, std::is_integral<Second>::value, First,
                                Second, Args...>::type type;
};

template<typename Range, typename First, typename Second, typename ...Args>
struct gpu_discard_range<true, Range, First, Second, Args...>{
  typedef typename gpu_arg_list<std::is_pointer<First>::value, std::is_integral<Second>::value, First,
                                Second,  Args...>::type type;
};

template<typename F, typename ...Args>
struct replace_pointer_size_pair_with_bufferptr{
  typedef typename gpu_discard_range<is_range<F>::value, F, Args...>::type type;
};

template<typename... Args> struct gpu_kernel_generator;

template<typename... Args>
struct gpu_kernel_generator<arg_list_gen<Args...>>
{
  typedef symphony::gpu_kernel<Args...> type;
};

template<bool is_output_buffer, typename... Args> struct output_buffer_type_extractor;

template<typename T, typename...Args>
struct output_buffer_type_extractor<true, T, Args...>{
  typedef T type;
};

template<typename F, typename S, typename...Args>
struct output_buffer_type_extractor<false, F, S, Args...>{
  typedef typename output_buffer_type_extractor<
    ::symphony::internal::pattern::utility::is_output_buffer_ptr<S>::value, S, Args...>::type type;
};

template<typename... Args> struct get_output_buffer_type;

template<typename F, typename... Args> struct get_output_buffer_type<arg_list_gen<F, Args...>>{
  typedef typename output_buffer_type_extractor<
    ::symphony::internal::pattern::utility::is_output_buffer_ptr<F>::value, F, Args...>::type type;
};

struct hexagon_arg_list {
typedef arg_list_gen<> type;
};

template<bool isRange, typename ...Args> struct hexagon_expand_range;

template<typename ...Args>
struct hexagon_expand_range<false,Args...> {
  typedef typename hexagon_arg_list::type::template
    append<Args...>::type type;
};

template<typename ...Args>
struct hexagon_expand_range<true, symphony::range<1>, Args...>{
  typedef typename hexagon_arg_list::type::template
    append<int, int, Args...>::type type;
};

template<typename ...Args>
struct hexagon_expand_range<true, symphony::range<2>, Args...>{
  typedef typename hexagon_arg_list::type::template
  append<int, int, int, int, Args...>::type type;
};

template<typename ...Args>
struct hexagon_expand_range<true, symphony::range<3>, Args...>{
  typedef typename hexagon_arg_list::type::template
    append<int, int, int, int, int, int, Args...>::type type;
};

template<typename F, typename ...Args>
struct make_hexagon_kernel_args{
  typedef typename hexagon_expand_range<is_range<F>::value, F, Args...>::type type;
};

template<typename... Args> struct hexagon_kernel_signature_generator;

template<typename... Args>
struct hexagon_kernel_signature_generator<arg_list_gen<Args...>>
{
  typedef int(*type)(Args...);
};

template<typename... Args> struct hexagon_kernel_generator;

template<typename... Args>
struct hexagon_kernel_generator<arg_list_gen<Args...>>
{
  typedef ::symphony::hexagon_kernel<int(*)(Args...)> type;
};

template<typename... Args> struct get_hexagon_output_buffer_type{
  typedef typename get_output_buffer_type<typename replace_pointer_size_pair_with_bufferptr<Args...>::type>::type type;
};

#ifdef SYMPHONY_HAVE_OPENCL
template<typename T = void>
static std::string stringize(std::string type, std::string name){
  std::string final_signature_string("");
  if(type.find("*") != std::string::npos){

    final_signature_string += "__global " + type;
  }
  else{

    final_signature_string += "__private " + type;
  }
  final_signature_string += " " + name + ")";
  return final_signature_string.c_str();
}

template<typename ...Args>
static std::string stringize(std::string type, std::string name, std::string type1, std::string name1, Args... args){
  std::string final_signature_string("");
  if(type.find("*") != std::string::npos){

    final_signature_string += "__global " + type;
  }
  else{

    final_signature_string += "__private " + type;
  }
  final_signature_string += " " + name + ", ";
  std::string temp =  final_signature_string + stringize(type1, name1, args...);
  return temp.c_str();
}

template<typename... Args>
std::string gpu_gen_signature(Args... args){

 std::string temp =  "(" + stringize(args...);
 return temp;
}
#endif
};

};

};
