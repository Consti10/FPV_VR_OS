// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>

#ifdef SYMPHONY_HAVE_RTTI
#include <typeinfo>
#endif

#ifdef _MSC_VER
#include <type_traits>
#endif

#include <symphony/internal/util/macros.hh>

namespace symphony {
namespace internal {

class demangler {

public:

#ifdef SYMPHONY_HAVE_RTTI
  static std::string demangle(std::type_info const& type) {
    return do_work(type);
  }
#endif

  template<typename... TS>
  static std::string demangle(){
#ifdef SYMPHONY_HAVE_RTTI
    std::string description(" ");
    demangle_traverse_types<TS...>(description,
                                   typename std::integral_constant<bool,(sizeof...(TS) != 0)>::type());
    return description;
#else
  return std::string("Enable RTTI for demangling class names.");
#endif
  }

private:

  template<typename... Types>
  static void demangle_traverse_types(std::string& description, std::true_type) {
    demangle_traverse_types_imp<Types...>(description);
  }

  template<typename... Types>
  static void demangle_traverse_types(std::string& , std::false_type) {
  }

  template<typename S1, typename S2, typename... Rest>
  static void demangle_traverse_types_imp(std::string& description) {
#ifdef SYMPHONY_HAVE_RTTI
    description += do_work(typeid(S1));
    description += ",";
    demangle_traverse_types_imp<S2, Rest...>(description);
#endif
    SYMPHONY_UNUSED(description);
  }

  template<typename S1>
  static void demangle_traverse_types_imp(std::string& description) {
#ifdef SYMPHONY_HAVE_RTTI
    description += do_work(typeid(S1));
#endif
    SYMPHONY_UNUSED(description);
  }

#ifdef SYMPHONY_HAVE_RTTI

  static std::string do_work(std::type_info const& type);
#endif

  SYMPHONY_DELETE_METHOD(demangler());
  SYMPHONY_DELETE_METHOD(demangler(demangler const&));
  SYMPHONY_DELETE_METHOD(demangler(demangler&&));
  SYMPHONY_DELETE_METHOD(demangler& operator=(demangler const&));
  SYMPHONY_DELETE_METHOD(demangler& operator=(demangler&&));
};

};
};
