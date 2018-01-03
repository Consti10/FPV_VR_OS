// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstddef>
#include <string>

#include <symphony/buffermode.hh>
#include <symphony/bufferiterator.hh>
#include <symphony/devicetypes.hh>
#include <symphony/memregion.hh>
#include <symphony/texturetype.hh>

#include <symphony/internal/buffer/buffer.hh>
#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/strprintf.hh>

namespace symphony {

template<typename T>
buffer_ptr<T> create_buffer(size_t num_elems)
{
  return create_buffer<T>(num_elems, buffer_mode::synchronized, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(size_t num_elems,
                            symphony::buffer_mode buf_mode)
{
  return create_buffer<T>(num_elems, buf_mode, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(size_t num_elems,
                            device_set const& likely_devices)
{
  return create_buffer<T>(num_elems, buffer_mode::synchronized, likely_devices);
}

template<typename T>
buffer_ptr<T> create_buffer(size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices);

template<typename T>
buffer_ptr<T> create_buffer(T* preallocated_ptr,
                            size_t num_elems)
{
  return create_buffer<T>(preallocated_ptr, num_elems, buffer_mode::synchronized, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(T* preallocated_ptr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode)
{
  return create_buffer<T>(preallocated_ptr, num_elems, buf_mode, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(T* preallocated_ptr,
                            size_t num_elems,
                            device_set const& likely_devices)
{
  return create_buffer<T>(preallocated_ptr, num_elems, buffer_mode::synchronized, likely_devices);
}

template<typename T>
buffer_ptr<T> create_buffer(T* preallocated_ptr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices);

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr)
{
  return create_buffer<T>(mr, 0, buffer_mode::synchronized, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            size_t num_elems)
{
  return create_buffer<T>(mr, num_elems, buffer_mode::synchronized, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            symphony::buffer_mode buf_mode)
{
  return create_buffer<T>(mr, 0, buf_mode, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            device_set const& likely_devices)
{
  return create_buffer<T>(mr, 0, buffer_mode::synchronized, likely_devices);
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode)
{
  return create_buffer<T>(mr, num_elems, buf_mode, device_set());
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            size_t num_elems,
                            device_set const& likely_devices)
{
  return create_buffer<T>(mr, num_elems, buffer_mode::synchronized, likely_devices);
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices)
{
  return create_buffer<T>(mr, 0, buf_mode, likely_devices);
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices);

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename T>
class buffer_ptr : private internal::buffer_ptr_base {

public:

  typedef buffer_iterator<T> iterator;

  typedef buffer_const_iterator<T> const_iterator;

  buffer_ptr() :
    base(),
    _num_elems(0)
  {}

  buffer_ptr(buffer_ptr< typename std::remove_const<T>::type > const& other) :
    base(reinterpret_cast<buffer_ptr_base const&>(other)),
    _num_elems(other.size())
  {}

  buffer_ptr& operator=(buffer_ptr< typename std::remove_const<T>::type > const& other)
  {
    if(static_cast<void const*>(this) == &other)
      return *this;

    base::operator=(reinterpret_cast<buffer_ptr_base const&>(other));
    _num_elems = other.size();
    return *this;
  }

  bool syncs_on_task_finish() const { return base::syncs_on_task_finish(); }

  void ro_sync() {
    base::ro_sync();
  }

  void w_invalidate() {
    base::w_invalidate();
  }

  void rw_sync() {
    base::rw_sync();
  }

  inline
  void* host_data() const {
    return base::host_data();
  }

  inline
  void* saved_host_data() const {
    return base::saved_host_data();
  }

#ifdef SYMPHONY_HAVE_OPENCL
  template<symphony::graphics::image_format img_format, int dims>
  buffer_ptr& treat_as_texture(symphony::graphics::image_size<dims> const& is) {
    base::treat_as_texture<img_format, dims>(is);
    return *this;
  }
#endif

  inline
  size_t size() const { return _num_elems; }

  inline
  T& operator[](size_t index) const {
    return reinterpret_cast<T*>(saved_host_data())[index];
  }

  inline
  T& at(size_t index) const {

    SYMPHONY_API_THROW(saved_host_data() != nullptr,
        "buffer data is currently not host accessible via this buffer_ptr");

    SYMPHONY_API_THROW_CUSTOM(0 <= index && index < _num_elems,
                              std::out_of_range,
                              "Out of bounds: index=%zu num_elems=%zu",
                              index,
                              _num_elems);

    return operator[](index);
  }

  iterator begin() const { return iterator(const_cast<buffer_ptr<T>*>(this), 0); }

  iterator end()   const { return iterator(const_cast<buffer_ptr<T>*>(this), size()); }

  const_iterator cbegin() const { return const_iterator(this, 0); }

  const_iterator cend()   const { return const_iterator(this, size()); }

  std::string to_string() const {
    std::string s = ::symphony::internal::strprintf("num_elems=%zu sizeof(T)=%zu ",
                                                _num_elems,
                                                sizeof(T));
    s.append(base::to_string());
    return s;
  }

private:

  typedef internal::buffer_ptr_base base;

  typedef typename std::remove_cv<T>::type bare_T;

  size_t _num_elems;

  buffer_ptr(size_t num_elems,
             symphony::buffer_mode buf_mode,
             device_set const& device_hints) :
    base(num_elems * sizeof(T),
         buf_mode,
         device_hints),
    _num_elems(num_elems)
  {}

  buffer_ptr(T* preallocated_ptr,
             size_t num_elems,
             symphony::buffer_mode buf_mode,
             device_set const& device_hints) :
    base(const_cast<bare_T*>(preallocated_ptr),
         num_elems * sizeof(T),
         buf_mode,
         device_hints),
    _num_elems(num_elems)
  {}

  buffer_ptr(memregion const& mr,
             size_t num_elems,
             symphony::buffer_mode buf_mode,
             device_set const& device_hints) :
    base(mr,
         num_elems > 0 ? num_elems * sizeof(T) : mr.get_num_bytes(),
         buf_mode,
         device_hints),
    _num_elems(num_elems > 0 ? num_elems : mr.get_num_bytes() / sizeof(T))
  {
    SYMPHONY_API_ASSERT(num_elems == 0 || num_elems * sizeof(T) <= mr.get_num_bytes(),
                    "num_elems=%zu exceed the capacity of the memory region size=%zu bytes",
                    num_elems, mr.get_num_bytes());
  }

  friend
  buffer_ptr<T> create_buffer<T>(size_t num_elems,
                                 symphony::buffer_mode buf_mode,
                                 device_set const& likely_devices);

  friend
  buffer_ptr<T> create_buffer<T>(T* preallocated_ptr,
                                 size_t num_elems,
                                 symphony::buffer_mode buf_mode,
                                 device_set const& likely_devices);

  friend
  buffer_ptr<T> create_buffer<T>(memregion const& mr,
                                 size_t num_elems,
                                 symphony::buffer_mode buf_mode,
                                 device_set const& likely_devices);

  inline
  bool is_null() const { return base::is_null(); }

  inline
  void const* get_buffer() const { return base::get_buffer(); }

  template<typename U>
  friend
  bool operator==(::symphony::buffer_ptr<U> const& b, ::std::nullptr_t);

  template<typename U>
  friend
  bool operator==(::std::nullptr_t, ::symphony::buffer_ptr<U> const& b);

  template<typename U>
  friend
  bool operator==(::symphony::buffer_ptr<U> const& b1, ::symphony::buffer_ptr<U> const& b2);

};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename T>
bool operator==(::symphony::buffer_ptr<T> const& b, ::std::nullptr_t) { return b.is_null(); }

template<typename T>
bool operator==(::std::nullptr_t, ::symphony::buffer_ptr<T> const& b) { return b.is_null(); }

template<typename T>
bool operator==(::symphony::buffer_ptr<T> const& b1, ::symphony::buffer_ptr<T> const& b2)
{
  return b1.get_buffer() == b2.get_buffer();
}

template<typename T>
bool operator!=(::symphony::buffer_ptr<T> const& b, ::std::nullptr_t) { return !(b == nullptr); }

template<typename T>
bool operator!=(::std::nullptr_t, ::symphony::buffer_ptr<T> const& b) { return !(nullptr == b); }

template<typename T>
bool operator!=(::symphony::buffer_ptr<T> const& b1, ::symphony::buffer_ptr<T> const& b2)
{
  return !(b1 == b2);
}

template<typename BufferPtr>
struct in;

template<typename BufferPtr>
struct out;

template<typename BufferPtr>
struct inout;

};
