// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if defined(SYMPHONY_HAVE_QTI_HEXAGON)

#include <utility>

#include <symphony/internal/buffer/bufferpolicy.hh>
#include <symphony/internal/task/task.hh>

namespace symphony {

template <typename Fn>
class hexagon_kernel;

namespace internal {

template<typename ...Params>
struct FnParams;

template<typename ...Params>
struct FnParams<int(*)(Params...)>
{
  using tuple_params = std::tuple<Params...>;
};

template <typename T>
struct is_buffer_ptr;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename T>
struct is_buffer_ptr : public std::false_type {};

template <typename T>
struct is_buffer_ptr<::symphony::buffer_ptr<T> > : public std::true_type {};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template <typename TP, size_t index, typename Arg>
struct match_buffer_against_two_Params;

template <typename TP, size_t index, typename T>
struct match_buffer_against_two_Params<TP, index, symphony::buffer_ptr<T>>
{
  static_assert( index + 1 < std::tuple_size<TP>::value, "Extra parameters found");
  static_assert( std::is_same<typename std::tuple_element<index, TP>::type, T>::value , "Incorrect buffer type");

  static_assert( std::is_integral<typename std::tuple_element<index+1, TP>::type >::value ,
      "Incorrect integral type after buffer");

  static constexpr auto next_index = index + 2;
};

template <typename TP, size_t index, typename Arg>
struct match_single_Arg_against_single_Param
{
  static constexpr auto next_index = index + 1;
};

template <typename TP, size_t index>
struct check_no_remaining_params
{
  static_assert( std::tuple_size<TP>::value == index, "Extra parameters found");
};

template <typename TP, size_t index, typename Arg, typename ...RemainingArgs>
struct compare_Args_against_Params
{
  typedef typename std::conditional<is_buffer_ptr<Arg>::value,
                    match_buffer_against_two_Params<TP, index, Arg>,
                    match_single_Arg_against_single_Param<TP, index, Arg>>::type arg_checker;

  typedef typename std::conditional< (sizeof...(RemainingArgs) > 0),
                    compare_Args_against_Params<TP, arg_checker::next_index, RemainingArgs...>,
                    check_no_remaining_params<TP, arg_checker::next_index>>
      ::type rest_args;
};

template<typename Fn, typename ...Args>
struct check;

template< typename Fn>
struct check<::symphony::hexagon_kernel<Fn>>
{
};

template <typename Fn , typename ...Args>
struct check<::symphony::hexagon_kernel<Fn>, Args...>
{
  using tuple_params = typename FnParams<Fn>::tuple_params;

  using checker = compare_Args_against_Params<tuple_params, 0, Args...>;
};

template<std::uint32_t ... numbers>
struct integer_sequence {

  static constexpr size_t get_size() {
    return sizeof...(numbers);
  }
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<std::uint32_t Top, std::uint32_t ...Sequence>
struct create_integer_sequence : create_integer_sequence<Top-1, Top-1, Sequence...> {
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<std::uint32_t ...Sequence>
struct create_integer_sequence<0, Sequence...> {
  using type = integer_sequence<Sequence...>;
};

template<typename F, typename ArgsTuple, uint32_t ...Number>
int apply_impl(F& f , ArgsTuple& args, integer_sequence<Number...>) {
  return f(std::get<Number>(args)...);
}

template<typename F, typename ArgsTuple>
int apply(F& f, ArgsTuple& args) {
  return apply_impl(f, args, typename create_integer_sequence<std::tuple_size<ArgsTuple>::value>::type());
}

template<typename T>
struct decay_and_unref {
  using type = typename std::remove_reference<typename std::decay<T>::type>::type;
};

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index, bool TP_index_is_buffer_ptr>
struct add_buffer_to_acquire_set;

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index>
struct add_buffer_to_acquire_set<BufferAcquireSet, TP, tp_index, Fn, fn_index, false> {
  add_buffer_to_acquire_set(BufferAcquireSet&, TP& ) {}
};

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index>
struct add_buffer_to_acquire_set<BufferAcquireSet, TP, tp_index, Fn, fn_index, true> {
  add_buffer_to_acquire_set(BufferAcquireSet& bas, TP& tp) {

    using param_type = typename std::tuple_element<tp_index, TP>::type;
    param_type param = std::get<tp_index>(tp);

    using fn_arg_type = typename std::tuple_element<fn_index, typename FnParams<Fn>::tuple_params>::type;
    static constexpr auto buffer_read_only = std::is_const<typename std::remove_pointer<fn_arg_type>::type>::value;
    auto acquire_mode =  buffer_read_only ?
      symphony::internal::bufferpolicy::acquire_r :
      symphony::internal::bufferpolicy::acquire_rw;

    bas.add(param, acquire_mode);
  }
};

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index, bool is_finished>
struct parse_and_add_buffers_to_acquire_set;

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index>
struct parse_and_add_buffers_to_acquire_set<BufferAcquireSet, TP, tp_index, Fn, fn_index, false> {
  using tuple_type = typename std::tuple_element<tp_index, TP>::type;
  using param_type = typename decay_and_unref<tuple_type>::type;

  static constexpr auto is_current_param_a_buffer_ptr = is_api20_buffer_ptr<param_type>::value;

  static constexpr size_t increment = is_current_param_a_buffer_ptr ? 2 : 1;
  using current_param_type = add_buffer_to_acquire_set<BufferAcquireSet,
                                                       TP,
                                                       tp_index,
                                                       Fn,
                                                       fn_index,
                                                       is_current_param_a_buffer_ptr>;

  current_param_type _current_param;

  parse_and_add_buffers_to_acquire_set<BufferAcquireSet,
                                       TP,
                                       tp_index + 1,
                                       Fn,
                                       fn_index + increment,
                                       tp_index + 1 >= std::tuple_size<TP>::value> _rest_params;

  parse_and_add_buffers_to_acquire_set(BufferAcquireSet& bas, TP& tp) :
    _current_param(bas, tp)
    , _rest_params(bas, tp)
  {}
};

template<typename BufferAcquireSet, typename TP, size_t tp_index, typename Fn, size_t fn_index>
struct parse_and_add_buffers_to_acquire_set<BufferAcquireSet, TP, tp_index, Fn, fn_index, true> {

  parse_and_add_buffers_to_acquire_set(BufferAcquireSet&, TP&) {}
};

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet, bool TP_index_is_buffer_ptr>
struct translate_single_TP_arg_to_hexagon_tuple;

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet>
struct translate_single_TP_arg_to_hexagon_tuple<Fn, fn_index, TP, index, BufferAcquireSet, false> {
  using arg_type = typename std::tuple_element<index, TP>::type;
  std::tuple<arg_type> _htp_at_index;

  translate_single_TP_arg_to_hexagon_tuple(TP& tp, BufferAcquireSet&) :
    _htp_at_index( std::get<index>(tp) )
  {
  }
};

template<typename BufferPtr>
struct buffertraits;

template<typename T>
struct buffertraits<::symphony::buffer_ptr<T>> {
  using element_type = T;
  using api20 = std::true_type;
};

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet>
struct translate_single_TP_arg_to_hexagon_tuple<Fn, fn_index, TP, index, BufferAcquireSet, true> {

  using buffer_type = typename std::decay<typename std::tuple_element<index, TP>::type>::type;

  using element_type = typename buffertraits<buffer_type>::element_type;
  std::tuple<element_type*, size_t> _htp_at_index;

  using fn_arg_type = typename std::tuple_element<fn_index, typename FnParams<Fn>::tuple_params>::type;

  template<typename T>
  element_type * get_data(::symphony::buffer_ptr<T> b, BufferAcquireSet& bas) {

    SYMPHONY_INTERNAL_ASSERT(bas.acquired(), "Buffers should be already acquired");

    auto acquired_arena = bas.find_acquired_arena(b);

    auto ion_ptr = arena_storage_accessor::access_ion_arena_for_hexagontask(acquired_arena);
    return static_cast<element_type *>(ion_ptr);
  }

  template<typename T>
  size_t get_size(::symphony::buffer_ptr<T> b) {
    return b.size();
  }

  translate_single_TP_arg_to_hexagon_tuple(TP& tp, BufferAcquireSet& bas) :
    _htp_at_index(std::make_tuple(get_data(std::get<index>(tp), bas), get_size(std::get<index>(tp))))
  { }
};

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet, bool is_finished>
struct translate_TP_args_to_hexagon;

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet>
struct translate_TP_args_to_hexagon<Fn, fn_index, TP, index, BufferAcquireSet, false> {
  using tuple_type = typename std::tuple_element<index, TP>::type;

  using element_type = typename decay_and_unref<tuple_type>::type;

  static bool constexpr is_index_arg_a_buffer_ptr =
                            ::symphony::internal::is_buffer_ptr<element_type>::value;

  translate_single_TP_arg_to_hexagon_tuple<Fn, fn_index, TP,
                                           index,
                                           BufferAcquireSet,
                                           is_index_arg_a_buffer_ptr
                                          > _hexagon_container_for_index;

  using rest_type = translate_TP_args_to_hexagon<Fn,
                               is_index_arg_a_buffer_ptr ? fn_index + 2 : fn_index + 1,
                               TP,
                               index+1,
                               BufferAcquireSet,
                               index+1 >=  std::tuple_size<TP>::value>;
  rest_type _rest;

  using hexagon_tuple_till_index = decltype( std::tuple_cat(_hexagon_container_for_index._htp_at_index,
                                                            _rest._htp_till_index) );

  hexagon_tuple_till_index _htp_till_index;

  translate_TP_args_to_hexagon(TP& tp, BufferAcquireSet& bas) :
    _hexagon_container_for_index(tp, bas)
    , _rest(tp, bas)
    , _htp_till_index( std::tuple_cat(_hexagon_container_for_index._htp_at_index, _rest._htp_till_index))
  {}
};

template<typename Fn, size_t fn_index, typename TP, size_t index, typename BufferAcquireSet>
struct translate_TP_args_to_hexagon<Fn, fn_index, TP, index, BufferAcquireSet, true> {
  std::tuple<> _htp_till_index;

  translate_TP_args_to_hexagon(TP&, BufferAcquireSet&) :
    _htp_till_index()
  {
  }
};

};
};

#endif
