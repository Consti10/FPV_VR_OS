// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>

#include <symphony/buffermode.hh>
#include <symphony/devicetypes.hh>
#include <symphony/memregion.hh>

#include <symphony/internal/buffer/bufferstate.hh>
#include <symphony/internal/buffer/memregionaccessor.hh>
#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/graphics/texture.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/symphonyptrs.hh>

namespace symphony {

template<typename T> class buffer_ptr;

};

namespace symphony {
namespace internal {

class buffer_accessor;

void* get_or_create_host_accessible_data_ptr(bufferstate* p_bufstate, bool make_host_data_valid);

void invalidate_non_host_arenas(bufferstate* p_bufstate);

void setup_preallocated_buffer_data(bufferstate* p_bufstate,
                                    void* preallocated_ptr,
                                    size_t size_in_bytes);

void setup_memregion_buffer_data(bufferstate* p_bufstate,
                                 internal_memregion* int_mr,
                                 size_t size_in_bytes);

using bufferstate_ptr = symphony::internal::symphony_shared_ptr<bufferstate>;

struct buffer_as_texture_info {
#ifdef SYMPHONY_HAVE_OPENCL
private:
  bool                          _used_as_texture;

  int                           _dims;
  symphony::graphics::image_format  _img_format;
  symphony::graphics::image_size<3> _img_size;

  symphony::graphics::internal::base_texture_cl* _tex_cl;

public:

  buffer_as_texture_info(bool used_as_texture                     = false,
                         int  dims                                = 0,
                         symphony::graphics::image_format  img_format = symphony::graphics::image_format::first,
                         symphony::graphics::image_size<3> img_size   = {0,0,0},
                         symphony::graphics::internal::base_texture_cl* tex_cl = nullptr) :
    _used_as_texture(used_as_texture),
    _dims(dims),
    _img_format(img_format),
    _img_size(img_size),
    _tex_cl(tex_cl)
  {}

  bool get_used_as_texture() const { return _used_as_texture; }
  int get_dims() const { return _dims; }
  symphony::graphics::image_format get_img_format() const { return _img_format; }
  symphony::graphics::image_size<3> get_img_size() const { return _img_size; }

  symphony::graphics::internal::base_texture_cl*& access_tex_cl() { return _tex_cl; }

#else
  buffer_as_texture_info() {}

  bool get_used_as_texture() const { return false; }
#endif
};

class buffer_ptr_base {

  bool _automatic_host_sync;

  bufferstate_ptr _bufstate;

  mutable void* _host_accessible_data;

  buffer_as_texture_info _tex_info;

  friend class buffer_accessor;

protected:

  buffer_ptr_base() :
    _automatic_host_sync(true),
    _bufstate(nullptr),
    _host_accessible_data(nullptr),
    _tex_info()
  {}

  buffer_ptr_base(size_t size_in_bytes,
                  symphony::buffer_mode buf_mode,
                  device_set const& device_hints) :
    _automatic_host_sync(buf_mode == buffer_mode::synchronized),
    _bufstate(new bufferstate(size_in_bytes, _automatic_host_sync, device_hints)),
    _host_accessible_data(nullptr),
    _tex_info()
  {
    if(_automatic_host_sync) {
      allocate_host_accessible_data(true);
      invalidate_non_host_arenas();
    }
  }

  buffer_ptr_base(void* preallocated_ptr,
                  size_t size_in_bytes,
                  symphony::buffer_mode buf_mode,
                  device_set const& device_hints) :
    _automatic_host_sync(buf_mode == buffer_mode::synchronized),
    _bufstate(new bufferstate(size_in_bytes, _automatic_host_sync, device_hints)),
    _host_accessible_data(nullptr),
    _tex_info()
  {
    SYMPHONY_API_ASSERT(preallocated_ptr != nullptr, "null preallocated region");
    setup_preallocated_buffer_data(::symphony::internal::c_ptr(_bufstate),
                                   preallocated_ptr,
                                   size_in_bytes);
    allocate_host_accessible_data(true);
    if(_automatic_host_sync)
      invalidate_non_host_arenas();
    SYMPHONY_INTERNAL_ASSERT(_host_accessible_data == preallocated_ptr,
                             "allocate_host_accessible_data() was expected to get back preallocated_ptr");
  }

  buffer_ptr_base(memregion const& mr,
                  size_t size_in_bytes,
                  symphony::buffer_mode buf_mode,
                  device_set const& device_hints) :
    _automatic_host_sync(buf_mode == buffer_mode::synchronized),
    _bufstate(new bufferstate(size_in_bytes, _automatic_host_sync, device_hints)),
    _host_accessible_data(nullptr),
    _tex_info()
  {
    auto int_mr = ::symphony::internal::memregion_base_accessor::get_internal_mr(mr);
    SYMPHONY_INTERNAL_ASSERT(int_mr != nullptr, "Error.Internal memregion object is null");
    SYMPHONY_INTERNAL_ASSERT(int_mr->get_type() != ::symphony::internal::memregion_t::none,
                             "Error. Invalid type of memregion");
    setup_memregion_buffer_data(::symphony::internal::c_ptr(_bufstate),
                                int_mr,
                                size_in_bytes);
    if(int_mr->get_type() == ::symphony::internal::memregion_t::main)
      allocate_host_accessible_data(true);
    if(_automatic_host_sync) {
      allocate_host_accessible_data(true);
      invalidate_non_host_arenas();
    }
  }

  SYMPHONY_DEFAULT_METHOD(buffer_ptr_base(buffer_ptr_base const&));
  SYMPHONY_DEFAULT_METHOD(buffer_ptr_base& operator=(buffer_ptr_base const&));
  SYMPHONY_DEFAULT_METHOD(buffer_ptr_base(buffer_ptr_base&&));
  SYMPHONY_DEFAULT_METHOD(buffer_ptr_base& operator=(buffer_ptr_base&&));

public:
  inline
  void* host_data() const {

    if(saved_host_data() != nullptr)
      return saved_host_data();

    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return nullptr;

    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    allocate_host_accessible_data(false);
    return saved_host_data();
  }

  inline
  void* saved_host_data() const { return _host_accessible_data; }

  inline
  bool syncs_on_task_finish() const {
    SYMPHONY_INTERNAL_ASSERT(_bufstate == nullptr ||
            _bufstate->does_automatic_host_sync() == _automatic_host_sync,
            "Inconsistent automatic host sync settings between bufferstate and buffer_ptr");
    return _automatic_host_sync;
  }

  void allocate_host_accessible_data(bool make_host_data_valid) const {
    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return;

    auto tmp = get_or_create_host_accessible_data_ptr( p_bufstate, make_host_data_valid );
    SYMPHONY_INTERNAL_ASSERT(_host_accessible_data == nullptr || _host_accessible_data == tmp,
                             "host accessible pointer must not change: was=%p now=%p",
                             _host_accessible_data,
                             tmp);
    _host_accessible_data = tmp;
    SYMPHONY_INTERNAL_ASSERT(_host_accessible_data != nullptr, "allocate_host_accessible_data failed");
  }

  void invalidate_non_host_arenas() {
    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return;

    ::symphony::internal::invalidate_non_host_arenas( p_bufstate );
  }

  inline
  void ro_sync() {
    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return;

    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    auto requestor = p_bufstate->get_any_confirmed_acquire_requestor();

    SYMPHONY_API_ASSERT(requestor == nullptr,
                        "Host access must not overlap with any task access: found requestor=%p",
                        requestor);

    allocate_host_accessible_data(true);
  }

  inline
  void w_invalidate() {
    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return;

    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    auto requestor = p_bufstate->get_any_confirmed_acquire_requestor();

    SYMPHONY_API_ASSERT(requestor == nullptr,
                        "Host access must not overlap with any task access: found requestor=%p",
                        requestor);

    invalidate_non_host_arenas();
    allocate_host_accessible_data(true);
  }

  inline
  void rw_sync() {
    auto p_bufstate = ::symphony::internal::c_ptr(_bufstate);
    if(p_bufstate == nullptr)
      return;

    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    auto requestor = p_bufstate->get_any_confirmed_acquire_requestor();

    SYMPHONY_API_ASSERT(requestor == nullptr,
                        "Host access must not overlap with any task access: found requestor=%p",
                        requestor);

    allocate_host_accessible_data(true);
    invalidate_non_host_arenas();
  }

  std::string to_string() const {
    std::string s = ::symphony::internal::strprintf("host_ptr=%p syncs=%s bufstate=%p",
                                                _host_accessible_data,
                                                _automatic_host_sync ? "Y" : "N",
                                                ::symphony::internal::c_ptr(_bufstate));
    return s;
  }

#ifdef SYMPHONY_HAVE_OPENCL
  template<symphony::graphics::image_format img_format, int dims>
  void treat_as_texture(symphony::graphics::image_size<dims> const& is) {
    _tex_info = buffer_as_texture_info(true,
                                       dims,
                                       img_format,
                                       symphony::graphics::internal::image_size_converter<3, dims>::convert(is));
  }
#endif

  inline
  bool is_null() const { return ::symphony::internal::c_ptr(_bufstate) == nullptr; }

  inline
  void const* get_buffer() const { return ::symphony::internal::c_ptr(_bufstate); }

};

class buffer_accessor {
public:
  static bufferstate_ptr get_bufstate(buffer_ptr_base& bb) {
    return bb._bufstate;
  }

  static bufferstate_ptr const get_bufstate(buffer_ptr_base const& bb) {
    return bb._bufstate;
  }

  static buffer_as_texture_info get_buffer_as_texture_info(buffer_ptr_base const& bb) {
    return bb._tex_info;
  }

  static void set_name_locked(buffer_ptr_base const& bb, std::string const& name) {
    auto p_bufstate = symphony::internal::c_ptr(bb._bufstate);
    SYMPHONY_API_ASSERT(p_bufstate != nullptr, "buffer_ptr does not point to a valid buffer");
    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    p_bufstate->set_name(name);
  }

  template<typename T>
  static void set_name_locked(buffer_ptr<T> const& b, std::string const& name) {
    set_name_locked(reinterpret_cast<buffer_ptr_base const&>(b), name);
  }

  static std::string get_state_string_locked(buffer_ptr_base const& bb) {
    auto p_bufstate = symphony::internal::c_ptr(bb._bufstate);
    SYMPHONY_API_ASSERT(p_bufstate != nullptr, "buffer_ptr does not point to a valid buffer");
    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    return p_bufstate->to_string();
  }

  template<typename T>
  static std::string get_state_string_locked(buffer_ptr<T> const& b) {
    return get_state_string_locked(reinterpret_cast<buffer_ptr_base const&>(b));
  }

  static void collect_statistics_locked(buffer_ptr_base const& bb,
                                        bool enable,
                                        bool print_at_buffer_dealloc = true)
  {
    auto p_bufstate = symphony::internal::c_ptr(bb._bufstate);
    SYMPHONY_API_ASSERT(p_bufstate != nullptr, "buffer_ptr does not point to a valid buffer");
    std::lock_guard<std::mutex> lock(p_bufstate->access_mutex());
    p_bufstate->collect_statistics(enable, print_at_buffer_dealloc);
  }

  template<typename T>
  static void collect_statistics_locked(buffer_ptr<T> const& b,
                                        bool enable,
                                        bool print_at_buffer_dealloc = true)
  {
    collect_statistics_locked(reinterpret_cast<buffer_ptr_base const&>(b), enable, print_at_buffer_dealloc);
  }

  SYMPHONY_DELETE_METHOD(buffer_accessor());
  SYMPHONY_DELETE_METHOD(buffer_accessor(buffer_accessor const&));
  SYMPHONY_DELETE_METHOD(buffer_accessor& operator=(buffer_accessor const&));
  SYMPHONY_DELETE_METHOD(buffer_accessor(buffer_accessor&&));
  SYMPHONY_DELETE_METHOD(buffer_accessor& operator=(buffer_accessor&&));
};

};
};

namespace symphony {

template<typename T>
buffer_ptr<T> create_buffer(size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices)
{
  SYMPHONY_API_ASSERT(num_elems > 0, "create_buffer(): cannot create empty buffer.");
  buffer_ptr<T> b(num_elems, buf_mode, likely_devices);
  return b;
}

template<typename T>
buffer_ptr<T> create_buffer(T* preallocated_ptr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices)
{
  SYMPHONY_API_ASSERT(num_elems > 0, "create_buffer(): cannot create empty buffer.");
  buffer_ptr<T> b(preallocated_ptr, num_elems, buf_mode, likely_devices);
  return b;
}

template<typename T>
buffer_ptr<T> create_buffer(memregion const& mr,
                            size_t num_elems,
                            symphony::buffer_mode buf_mode,
                            device_set const& likely_devices)
{
  SYMPHONY_API_ASSERT(mr.get_num_bytes() > 0 || num_elems > 0, "create_buffer(): cannot create empty buffer.");
  buffer_ptr<T> b(mr,
                  num_elems,
                  buf_mode,
                  likely_devices);
  return b;
}

};

namespace symphony {

template<typename BufferPtr>
struct in {
  using buffer_type = BufferPtr;
};

template<typename BufferPtr>
struct out {
  using buffer_type = BufferPtr;
};

template<typename BufferPtr>
struct inout {
  using buffer_type = BufferPtr;
};

};
