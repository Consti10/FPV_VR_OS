// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <type_traits>

namespace symphony {

namespace internal {

template <typename... TS>
struct count_typenames;

template<>
struct count_typenames<> {
  enum {value = 0 };
};

template<typename T, typename... TS>
struct count_typenames<T, TS...> {
  enum {value = 1 + count_typenames<TS...>::value};
};

template <typename T, typename... TS>
struct type_exists;

template<typename T>
struct type_exists<T> {
  enum {value = 0};
};

template<typename T, typename T2, typename... TS>
struct type_exists<T, T2, TS...> {
  enum {
    value = ((std::is_same<T, T2>::value == 1) ?
             1 : type_exists<T, TS...>::value) };
};

template <typename... TS>
struct duplicated_types;

template<>
struct duplicated_types<> {
  enum {value = 0};
};

template <typename T, typename... TS>
struct duplicated_types<T, TS...> {
  enum {
    value = ((type_exists<T, TS...>::value == 1) ?
             1 : duplicated_types<TS...>::value) };
};

template<typename T>
class has_operator_parenthesis {
private:

  template <typename U>
    static auto check(int) -> decltype(&U::operator(), std::true_type());

  template <typename>
  static std::false_type check(...);

  typedef decltype(check<T>(0)) enable_if_type;

public:

  typedef typename enable_if_type::type type;

  enum{
    value = type::value
  };

};

template<typename A, typename B>
struct larger_type {
  typedef typename std::conditional<(sizeof(A) > sizeof(B)), A, B>::type type;
};

template <size_t... N>
struct integer_list_gen {
  template <size_t M>
  struct append {
    typedef integer_list_gen<N..., M> type;
  };
};

template <size_t N>
struct integer_list {
  typedef typename integer_list<N - 1>::type::template append<N>::type type;
};

template <>
struct integer_list<0> {
  typedef integer_list_gen<> type;
};

template <size_t N, size_t Offset>
struct integer_list_offset {
  typedef typename integer_list_offset<N - 1, Offset>::type::template append<N+Offset>::type type;
};

template <size_t Offset>
struct integer_list_offset<0, Offset> {
  typedef integer_list_gen<> type;
};

template <size_t... Indices, typename Tuple>
auto tuple_subset(Tuple& t, integer_list_gen<Indices...>)
-> decltype(std::make_tuple(std::get<Indices>(t)...))
{
  return std::make_tuple(std::get<Indices>(t)...);
}

template <size_t... Indices, typename Tuple>
auto tuple_head(Tuple& t, integer_list_gen<Indices...>)
-> decltype(std::make_tuple(std::get<Indices-1>(t)...))
{
  return std::make_tuple(std::get<Indices-1>(t)...);
}

template <typename First, typename... Rest>
std::tuple<Rest...> tuple_rest(std::tuple<First, Rest...>& t)
{
  return tuple_subset(t, typename integer_list<sizeof...(Rest)>::type());
}

template<typename F, typename TP, size_t prev_index, typename ...ExpandedArgs>
struct apply_tuple_helper {
  static void expand(F& f, TP&& tp, ExpandedArgs&& ...eargs) {
    using bare_TP = typename std::remove_reference<typename std::remove_cv<TP>::type>::type;
    using Arg = typename std::tuple_element<prev_index-1, bare_TP>::type;
    apply_tuple_helper<F, TP, prev_index-1, Arg, ExpandedArgs...>::expand(f,
                                                                          std::forward<TP>(tp),
                                                                          std::forward<Arg>(std::get<prev_index-1>(tp)),
                                                                          std::forward<ExpandedArgs>(eargs)... );
  }
};

template<typename F, typename TP, typename ...ExpandedArgs>
struct apply_tuple_helper<F, TP, 0, ExpandedArgs...> {
  static void expand(F& f, TP&& , ExpandedArgs&& ...eargs) {
    f(std::forward<ExpandedArgs>(eargs)...);
  }
};

template<typename F, typename TP>
void apply_tuple(F& f, TP&& tp) {
  using bare_TP = typename std::remove_reference<typename std::remove_cv<TP>::type>::type;
  size_t constexpr prev_index = std::tuple_size<bare_TP>::value;
  apply_tuple_helper<F, TP, prev_index>::expand(f, std::forward<TP>(tp));
}

};
};
