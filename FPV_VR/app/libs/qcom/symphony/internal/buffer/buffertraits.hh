// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

namespace symphony{

namespace internal{

namespace buffer {

namespace utility{

template<typename BufferPtr>
struct buffertraits;

template<typename T>
struct buffertraits<::symphony::buffer_ptr<T>> {
  using element_type = T;
  using api20 = std::true_type;
};

template <typename T>
struct is_buffer_ptr;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename T>
struct is_buffer_ptr : public std::false_type {};

template <typename T>
struct is_buffer_ptr<::symphony::buffer_ptr<T> > : public std::true_type {};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename TP, size_t index, bool TP_index_is_buffer_ptr>
struct expand_buffer_to_pointer_size_pair_for_cpu;

template<typename TP, size_t index>
struct expand_buffer_to_pointer_size_pair_for_cpu<TP, index, false> {
  using arg_type = typename std::tuple_element<index, TP>::type;
  std::tuple<arg_type> _tp_at_index;

  explicit expand_buffer_to_pointer_size_pair_for_cpu(TP& tp) :
    _tp_at_index( std::get<index>(tp) )
  {
  }
};

template <typename... Args>
struct type_list;

template <typename... Args>
struct type_list<symphony::gpu_kernel<Args...>>
{
   template <std::size_t N>
   using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
};

template<typename T, T V>
struct constant{
  constexpr  operator T() const { return V; }
};

template<typename TP, size_t index>
struct expand_buffer_to_pointer_size_pair_for_cpu<TP, index, true> {

  using buffer_type = typename std::decay<typename std::tuple_element<index, TP>::type>::type;

  using element_type = typename buffertraits<buffer_type>::element_type;
  std::tuple<element_type*, size_t> _tp_at_index;

  template<typename T>
  element_type * get_data(::symphony::buffer_ptr<T> b){
    auto bptr = static_cast<element_type*>(b.host_data());
    if(bptr == nullptr){
      b.ro_sync();
      bptr = static_cast<element_type*>(b.host_data());
    }
    return bptr;
  }

  template<typename T>
  size_t get_size(::symphony::buffer_ptr<T> b) {
    return b.size();
  }

  explicit expand_buffer_to_pointer_size_pair_for_cpu(TP& tp) :
    _tp_at_index(std::make_tuple(get_data(std::get<index>(tp)), get_size(std::get<index>(tp))))
  { }
};

template<typename TP, size_t index, bool is_finished>
struct expand_buffers_in_args_for_cpu;

template<typename TP, size_t index>
struct expand_buffers_in_args_for_cpu<TP, index, false> {
  using tuple_type = typename std::tuple_element<index, TP>::type;

  using element_type = typename std::decay<tuple_type>::type;

  static bool constexpr is_index_arg_a_buffer_ptr =
                            is_buffer_ptr<element_type>::value;

  expand_buffer_to_pointer_size_pair_for_cpu<TP,
                                    index,
                                    is_index_arg_a_buffer_ptr
                                    > _cpu_container_for_index;

  using rest_type = expand_buffers_in_args_for_cpu<TP,
                               index+1,
                               index+1 >=  std::tuple_size<TP>::value>;
  rest_type _rest;

  using cpu_tuple_till_index = decltype( std::tuple_cat(_cpu_container_for_index._tp_at_index,
                                                            _rest._expanded_args_list) );

  cpu_tuple_till_index _expanded_args_list;

  explicit expand_buffers_in_args_for_cpu(TP& tp) :
    _cpu_container_for_index(tp)
    , _rest(tp)
    , _expanded_args_list( std::tuple_cat(_cpu_container_for_index._tp_at_index, _rest._expanded_args_list))
  {}
};

template<typename TP, size_t index>
struct expand_buffers_in_args_for_cpu< TP, index, true> {
  std::tuple<> _expanded_args_list;

  explicit expand_buffers_in_args_for_cpu(TP&) :
    _expanded_args_list()
  {
  }
};

template<typename TP, size_t index, bool TP_index_is_buffer_ptr>
struct expand_buffer_to_buffer_size_pair_for_gpu;

template<typename TP, size_t index>
struct expand_buffer_to_buffer_size_pair_for_gpu<TP, index, false> {
  using arg_type = typename std::tuple_element<index, TP>::type;
  std::tuple<arg_type> _tp_at_index;

  explicit expand_buffer_to_buffer_size_pair_for_gpu(TP& tp) :
    _tp_at_index( std::get<index>(tp) )
  {
  }
};

template<typename TP, size_t index>
struct expand_buffer_to_buffer_size_pair_for_gpu<TP, index, true> {
  using tuple_type = typename std::tuple_element<index, TP>::type;

  std::tuple<tuple_type, size_t> _tp_at_index;

  template<typename T>
  size_t get_size(::symphony::buffer_ptr<T> b) {
    return b.size();
  }

  explicit expand_buffer_to_buffer_size_pair_for_gpu(TP& tp) :
    _tp_at_index(std::make_tuple(std::get<index>(tp), get_size(std::get<index>(tp))))
  { }
};

template<typename TP, size_t index, bool is_finished>
struct expand_buffers_in_args_for_gpu;

template<typename TP, size_t index>
struct expand_buffers_in_args_for_gpu<TP, index, false> {
  using tuple_type = typename std::tuple_element<index, TP>::type;

  using element_type = typename std::decay<tuple_type>::type;

  static bool constexpr is_index_arg_a_buffer_ptr =
                            is_buffer_ptr<element_type>::value;

  expand_buffer_to_buffer_size_pair_for_gpu<TP,
                                    index,
                                    is_index_arg_a_buffer_ptr
                                    > _gpu_container_for_index;

  using rest_type = expand_buffers_in_args_for_gpu<TP,
                               index+1,
                               index+1 >=  std::tuple_size<TP>::value>;
  rest_type _rest;

  using gpu_tuple_till_index = decltype( std::tuple_cat(_gpu_container_for_index._tp_at_index,
                                                            _rest._expanded_args_list) );

  gpu_tuple_till_index _expanded_args_list;

  explicit expand_buffers_in_args_for_gpu(TP& tp) :
    _gpu_container_for_index(tp)
    , _rest(tp)
    , _expanded_args_list( std::tuple_cat(_gpu_container_for_index._tp_at_index, _rest._expanded_args_list))
  {}
};

template<typename TP, size_t index>
struct expand_buffers_in_args_for_gpu<TP, index, true> {
  std::tuple<> _expanded_args_list;

  explicit expand_buffers_in_args_for_gpu(TP&) :
    _expanded_args_list()
  {
  }
};

};
};
};
};
