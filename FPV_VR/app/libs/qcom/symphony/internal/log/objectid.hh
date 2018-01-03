// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <cstdint>

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

namespace symphony {
namespace internal {
namespace log {

template <typename ObjectType> class null_object_id;
template <typename ObjectType> class seq_object_id;

template <typename ObjectType>
class object_id_base {

public:
  typedef std::uint32_t raw_id_type;
  typedef ObjectType object_type;

  raw_id_type get_raw_value() const {
    static_assert(sizeof(null_object_id<object_type>) == sizeof(seq_object_id<object_type>),
                  "null_object_id and seq_object_id are of differnt sizes");
    return _id;
  }

protected:

  enum commom_ids : raw_id_type {
    s_invalid_object_id = 0,
    s_first_valid_object_id =  1
  };

  object_id_base() : _id(s_invalid_object_id) { }
  explicit object_id_base(raw_id_type id) : _id(id) {
    SYMPHONY_INTERNAL_ASSERT(id >= s_first_valid_object_id, "Invalid object id.");
  }

private:

  const raw_id_type _id;
};

template <typename ObjectType>
class null_object_id : public object_id_base<ObjectType> {

  typedef object_id_base<ObjectType> parent;

public:

  null_object_id():parent() {}

};

template <typename ObjectType>
class seq_object_id : public object_id_base<ObjectType> {

  typedef object_id_base<ObjectType> parent;

public:

  typedef typename parent::raw_id_type raw_id_type;

  seq_object_id()
    :parent(s_counter.fetch_add(1, symphony::mem_order_relaxed)) {}

  seq_object_id(seq_object_id const& other)
    :parent(other.get_raw_value()) {}

  seq_object_id(null_object_id<ObjectType> const&)
    :parent() {}

private:

  static std::atomic<raw_id_type> s_counter;
};

template<typename ObjectType>
std::atomic<typename object_id_base<ObjectType>::raw_id_type>
  seq_object_id<ObjectType>::
  s_counter(object_id_base<ObjectType>::s_first_valid_object_id);

};
};
};

SYMPHONY_GCC_IGNORE_END("-Weffc++");
