// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <tuple>
#include <type_traits>

#include <symphony/internal/buffer/buffer.hh>
#include <symphony/internal/buffer/bufferstate.hh>
#include <symphony/internal/compat/compat.h>
#include <symphony/internal/compat/compilercompat.h>
#include <symphony/internal/task/task.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/strprintf.hh>

namespace symphony {
namespace internal {

class arena;

class bufferpolicy {

public:

  enum action_t {
    acquire_r,
    acquire_w,
    acquire_rw,
    release
  };

  virtual ~bufferpolicy(){}

  virtual arena_t get_arena_type_accessed_by_device(executor_device ed) = 0;

  virtual arena* unsafe_get_or_create_cpu_arena(bufferstate* bufstate,
                                                size_t size_in_bytes,
                                                bool make_data_valid) = 0;

  virtual arena* unsafe_create_arena_with_cpu_initial_data(bufferstate* bufstate,
                                                           void* p,
                                                           size_t size_in_bytes) = 0;

  virtual arena* unsafe_create_arena_with_memregion_initial_data(bufferstate* bufstate,
                                                                 internal_memregion* int_mr,
                                                                 size_t size_in_bytes) = 0;

  enum class acquire_scope {

    tentative,

    confirm,

    full
  };

  struct conflict_info {
    bool                    _no_conflict_found;
    void const*             _conflicting_requestor;
    per_device_arena_array  _acquired_arena_per_device;

    conflict_info() :
      _no_conflict_found(true),
      _conflicting_requestor(nullptr),
      _acquired_arena_per_device()
    {}

    conflict_info(bool no_conflict_found,
                  void const* conflicting_requestor) :
      _no_conflict_found(no_conflict_found),
      _conflicting_requestor(conflicting_requestor),
      _acquired_arena_per_device()
    {}

    std::string to_string() const {
      std::string arenas_string = std::string("[");
      for(auto i=static_cast<size_t>(executor_device::first); i <= static_cast<size_t>(executor_device::last); i++) {
        arenas_string.append(symphony::internal::strprintf("%p", _acquired_arena_per_device[i]));
        if(i < static_cast<size_t>(executor_device::last))
          arenas_string.append(", ");
      }
      arenas_string.append("]");
      return symphony::internal::strprintf("{%s, arenas=%s, conflicting_requestor=%p}",
                                       (_no_conflict_found ? "no_conflict" : "conflict"),
                                       arenas_string.c_str(),
                                       _conflicting_requestor);
    }
  };

  virtual conflict_info request_acquire_action(bufferstate* bufstate,
                                               void const* requestor,
                                               executor_device_bitset const& edb,
                                               action_t ac,
                                               acquire_scope as,
                                               buffer_as_texture_info tex_info,
                                               bool lock_bufstate = true) = 0;

  virtual void release_action(bufferstate* bufstate,
                              void const* requestor,
                              bool lock_bufstate = true) = 0;

  virtual void remove_matching_arena(bufferstate* bufstate,
                                     executor_device ed,
                                     bool lock_bufstate = true) = 0;

};

inline std::string to_string(bufferpolicy::action_t ac) {
  switch(ac) {
  case bufferpolicy::acquire_r:  return "acqR";
  case bufferpolicy::acquire_w:  return "acqW";
  case bufferpolicy::acquire_rw: return "acqRW";
  case bufferpolicy::release:    return "rel";
  default: return "invalid";
  }
}

bufferpolicy* get_current_bufferpolicy();

template<typename T>
struct strip_toplevel {
  using type = typename std::remove_cv< typename std::remove_reference< typename std::remove_cv<T>::type >::type >::type;
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template <typename T>
struct is_api20_buffer_ptr_helper;

template<typename T>
struct is_api20_buffer_ptr_helper: public std::false_type {};

template <typename T>
struct is_api20_buffer_ptr_helper< ::symphony::buffer_ptr<T> > : public std::true_type {};

template <typename T>
struct is_api20_buffer_ptr_helper< ::symphony::in<::symphony::buffer_ptr<T>> > : public std::true_type {};

template <typename T>
struct is_api20_buffer_ptr_helper< ::symphony::out<::symphony::buffer_ptr<T>> > : public std::true_type {};

template <typename T>
struct is_api20_buffer_ptr_helper< ::symphony::inout<::symphony::buffer_ptr<T>> > : public std::true_type {};

template<typename T>
struct is_api20_buffer_ptr : public is_api20_buffer_ptr_helper< typename strip_toplevel<T>::type > {};

template<typename BufPtr>
struct is_const_buffer_ptr_helper : public std::false_type {};

template<typename T>
struct is_const_buffer_ptr_helper< ::symphony::buffer_ptr<const T> > : public std::true_type {};

template<typename T>
struct is_const_buffer_ptr : public is_const_buffer_ptr_helper< typename strip_toplevel<T>::type > {};

SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename Tuple, size_t index>
struct num_buffer_ptrs_in_tuple_helper {
  using elem = typename std::tuple_element<index-1, Tuple>::type;
  enum { value =  (is_api20_buffer_ptr<elem>::value ? 1 : 0) +
                  num_buffer_ptrs_in_tuple_helper<Tuple, index-1>::value };
};

template<typename Tuple>
struct num_buffer_ptrs_in_tuple_helper<Tuple, 0> {
  enum { value = 0 };
};

template<typename Tuple>
struct num_buffer_ptrs_in_tuple {
  enum { value = num_buffer_ptrs_in_tuple_helper<Tuple, std::tuple_size<Tuple>::value>::value };
};

template<typename ...Args>
struct num_buffer_ptrs_in_args;

template<typename Arg, typename ...Args>
struct num_buffer_ptrs_in_args<Arg, Args...> {
  enum { value = (is_api20_buffer_ptr<Arg>::value ? 1 : 0) +
                  num_buffer_ptrs_in_args<Args...>::value };
};

template<>
struct num_buffer_ptrs_in_args<> {
  enum { value = 0 };
};

template<typename BuffersArrayType>
struct checked_addition_of_buffer_entry;

template<typename BufferInfo>
struct checked_addition_of_buffer_entry<std::vector<BufferInfo>> {
  static void add(std::vector<BufferInfo>& arr_buffers,
                  size_t&                  num_buffers_added,
                  BufferInfo const&        bi)
  {
    SYMPHONY_INTERNAL_ASSERT(arr_buffers.size() == num_buffers_added, "new entry expected to be added only at end");
    arr_buffers.push_back(bi);
    num_buffers_added = arr_buffers.size();
  }
};

template<typename BufferInfo, size_t MaxNumBuffers>
struct checked_addition_of_buffer_entry<std::array<BufferInfo, MaxNumBuffers>> {
  static void add(std::array<BufferInfo, MaxNumBuffers>& arr_buffers,
                  size_t&                                num_buffers_added,
                  BufferInfo const&                      bi)
  {
    SYMPHONY_INTERNAL_ASSERT(num_buffers_added < MaxNumBuffers,
                         "buffer_acquire_set: adding beyond expected max size: _num_buffers_added=%zu must be < MaxNumBuffers=%zu",
                         num_buffers_added,
                         MaxNumBuffers);

    arr_buffers[num_buffers_added++] = bi;
  }
};

template<typename ArenasArrayType>
struct resize_arenas_array {
  static void resize(size_t           arr_buffers_size,
                     ArenasArrayType& acquired_arenas)

  {
    SYMPHONY_INTERNAL_ASSERT(arr_buffers_size == acquired_arenas.size(),
                         "Fixed-size acquired_arenas (size=%zu) must have same size as final arr_buffers.size()=%zu",
                         acquired_arenas.size(),
                         arr_buffers_size);
    for(auto& ma : acquired_arenas)
      for(auto& a : ma)
        a = nullptr;
  }
};

template<size_t MultiDeviceCount>
struct resize_arenas_array< std::vector<std::array<arena*, MultiDeviceCount>> > {
  static void resize(size_t                                             arr_buffers_size,
                     std::vector<std::array<arena*, MultiDeviceCount>>& acquired_arenas)
  {
    acquired_arenas.resize(arr_buffers_size);
    for(auto& ma : acquired_arenas)
      for(auto& a : ma)
        a = nullptr;
  }
};

template<typename Entity>
class buffer_entity_finder {

  bool _found;

  Entity _entity;

  size_t _index;

public:

  buffer_entity_finder() :
    _found(false),
    _entity(),
    _index(0)
  {}

  buffer_entity_finder(Entity found_entity,
                       size_t found_index) :
    _found(true),
    _entity(found_entity),
    _index(found_index)
  {}

  bool found() const { return _found; }

  Entity entity() const {
    SYMPHONY_INTERNAL_ASSERT(_found, "No entity has been found.");
    return _entity;
  }

  size_t index() const {
    SYMPHONY_INTERNAL_ASSERT(_found, "No entity has been found.");
    return _index;
  }

  SYMPHONY_DEFAULT_METHOD(buffer_entity_finder(buffer_entity_finder const&));
  SYMPHONY_DEFAULT_METHOD(buffer_entity_finder& operator=(buffer_entity_finder const&));
  SYMPHONY_DEFAULT_METHOD(buffer_entity_finder(buffer_entity_finder&&));
  SYMPHONY_DEFAULT_METHOD(buffer_entity_finder& operator=(buffer_entity_finder&&));
};

template<typename Entity, bool FixedSize, size_t MaxNumEntities = 0>
class buffer_entity_container;

template<typename Entity, size_t MaxNumEntities>
class buffer_entity_container<Entity, true, MaxNumEntities>
{
  using container = std::array<std::pair<bufferstate*, Entity>, MaxNumEntities>;

  std::unique_ptr<container> _container;

public:
  buffer_entity_container() :
    _container(nullptr)
  {}

  bool add_buffer_entity(bufferstate* bufstate,
                         Entity&      entity)
  {
    SYMPHONY_INTERNAL_ASSERT(bufstate != nullptr, "null buffer");
    if(_container == nullptr) {
      _container.reset(new container);
      for(auto& p: *_container)
        p.first = nullptr;
    }

    bool saved = false;
    for(auto& p: *_container) {
      if(p.first == nullptr) {
        p = std::make_pair(bufstate, entity);
        saved = true;
        break;
      }
    }
    return saved;
  }

  buffer_entity_finder<Entity> find_buffer_entity(bufferstate* bufstate, size_t start_index = 0) const {
    SYMPHONY_INTERNAL_ASSERT(bufstate != nullptr, "null buffer");
    if(_container == nullptr)
      return buffer_entity_finder<Entity>();
    for(size_t i=start_index; i<(*_container).size(); i++) {
      auto const& p = (*_container)[i];
      if(p.first == bufstate)
        return buffer_entity_finder<Entity>(p.second, i);
      if(p.first == nullptr)
        return buffer_entity_finder<Entity>();
    }
    return buffer_entity_finder<Entity>();
  }

  bool has_any() const {
    return (_container != nullptr);
  }

  SYMPHONY_DELETE_METHOD(buffer_entity_container(buffer_entity_container const&));
  SYMPHONY_DELETE_METHOD(buffer_entity_container& operator=(buffer_entity_container const&));

  SYMPHONY_DEFAULT_METHOD(buffer_entity_container(buffer_entity_container&&));
  SYMPHONY_DEFAULT_METHOD(buffer_entity_container& operator=(buffer_entity_container&&));
};

template<typename Entity, size_t MaxNumEntities>
class buffer_entity_container<Entity, false, MaxNumEntities>
{
  using container = std::vector<std::pair<bufferstate*, Entity>>;

  container _container;

public:
  buffer_entity_container() :
    _container()
  {}

  bool add_buffer_entity(bufferstate* bufstate,
                         Entity&      entity)
  {
    SYMPHONY_INTERNAL_ASSERT(bufstate != nullptr, "null buffer");
    _container.push_back( std::make_pair(bufstate, entity) );
    return true;
  }

  buffer_entity_finder<Entity> find_buffer_entity(bufferstate* bufstate, size_t start_index = 0) const {
    SYMPHONY_INTERNAL_ASSERT(bufstate != nullptr, "null buffer");
    for(size_t i=start_index; i<_container.size(); i++) {
      auto const& p = _container[i];
      if(p.first == bufstate)
        return buffer_entity_finder<Entity>(p.second, i);
      if(p.first == nullptr)
        return buffer_entity_finder<Entity>();
    }
    return buffer_entity_finder<Entity>();
  }

  bool has_any() const {
    return (!_container.empty());
  }

  SYMPHONY_DELETE_METHOD(buffer_entity_container(buffer_entity_container const&));
  SYMPHONY_DELETE_METHOD(buffer_entity_container& operator=(buffer_entity_container const&));

  SYMPHONY_DEFAULT_METHOD(buffer_entity_container(buffer_entity_container&&));
  SYMPHONY_DEFAULT_METHOD(buffer_entity_container& operator=(buffer_entity_container&&));
};

using preacquired_arena_finder = buffer_entity_finder<arena*>;

class preacquired_arenas_base {
public:

  virtual void register_preacquired_arena(bufferstate* bufstate, arena* preacquired_arena) = 0;

  virtual preacquired_arena_finder find_preacquired_arena(bufferstate* bufstate, size_t start_index = 0) const = 0;

  virtual bool has_any() const = 0;

  virtual ~preacquired_arenas_base() {}
};

template<bool FixedSize, size_t MaxNumArenas = 0>
class preacquired_arenas : public preacquired_arenas_base
{
  buffer_entity_container<arena*, FixedSize, MaxNumArenas> _bec;

public:
  preacquired_arenas() :
    _bec()
  {}

  void register_preacquired_arena(bufferstate* bufstate, arena* preacquired_arena)
  {
    SYMPHONY_INTERNAL_ASSERT(preacquired_arena != nullptr, "null arena");

    bool saved = _bec.add_buffer_entity(bufstate, preacquired_arena);
    SYMPHONY_API_ASSERT(saved, "unable to save preacquired_arena=%p for bufstate=%p",
                        preacquired_arena,
                        bufstate);
  }

  preacquired_arena_finder find_preacquired_arena(bufferstate* bufstate, size_t start_index = 0) const {
    return _bec.find_buffer_entity(bufstate, start_index);
  }

  bool has_any() const {
    return _bec.has_any();
  }

  SYMPHONY_DELETE_METHOD(preacquired_arenas(preacquired_arenas const&));
  SYMPHONY_DELETE_METHOD(preacquired_arenas& operator=(preacquired_arenas const&));

  SYMPHONY_DEFAULT_METHOD(preacquired_arenas(preacquired_arenas&&));
  SYMPHONY_DEFAULT_METHOD(preacquired_arenas& operator=(preacquired_arenas&&));
};

using override_device_set_finder = buffer_entity_finder<executor_device_bitset>;

class override_device_sets_base {
public:

  virtual void register_override_device_set(bufferstate* bufstate, executor_device_bitset edb) = 0;

  virtual override_device_set_finder find_override_device_set(bufferstate* bufstate) const = 0;

  virtual ~override_device_sets_base() {}
};

template<bool FixedSize, size_t MaxNumBuffers = 0>
class override_device_sets : public override_device_sets_base
{
  buffer_entity_container<executor_device_bitset, FixedSize, MaxNumBuffers> _bec;

public:
  override_device_sets() :
    _bec()
  {}

  void register_override_device_set(bufferstate* bufstate, executor_device_bitset override_edb)
  {
    SYMPHONY_INTERNAL_ASSERT(!override_edb.has(executor_device::unspecified) && override_edb.count() > 0,
                             "Need at least one valid device in override_edb=%s",
                             internal::to_string(override_edb).c_str());

    SYMPHONY_INTERNAL_ASSERT(_bec.find_buffer_entity(bufstate).found() == false,
        "Only one override executor_device_bitset may be specified for a buffer: bufstate=%p already contains one",
        bufstate);

    bool saved = _bec.add_buffer_entity(bufstate, override_edb);
    SYMPHONY_API_ASSERT(saved, "unable to save override edb=%s for bufstate=%p",
                        internal::to_string(override_edb).c_str(),
                        bufstate);
  }

  override_device_set_finder find_override_device_set(bufferstate* bufstate) const {
    auto odf = _bec.find_buffer_entity(bufstate, 0);

    SYMPHONY_INTERNAL_ASSERT(!odf.found() ||
        _bec.find_buffer_entity(bufstate, odf.index()+1).found() == false,
        "Only one override executor_device_bitset may be specified for a buffer: found multiple for bufstate=%p",
        bufstate);
    return odf;
  }

  bool has_any() const {
    return _bec.has_any();
  }

  SYMPHONY_DELETE_METHOD(override_device_sets(override_device_sets const&));
  SYMPHONY_DELETE_METHOD(override_device_sets& operator=(override_device_sets const&));
};

template<size_t MultiDeviceCount>
std::string to_string(std::array<symphony::internal::executor_device, MultiDeviceCount> const& multi_ed)
{
  std::string s = std::string("[");
  for(auto ed: multi_ed)
    s.append(internal::to_string(ed) + " ");
  s.append( "]");
  return s;
}

enum class acquire_status {
  idle,
  tentatively_acquired,
  fully_acquired
};

template<size_t MaxNumBuffers, bool FixedSize = true, size_t MultiDeviceCount = 1>
class buffer_acquire_set {
  using action_t = symphony::internal::bufferpolicy::action_t;

  static constexpr size_t s_flag_tentative_acquire = 0x1;

  static constexpr size_t s_flag_fake_arena = 0x2;

  struct buffer_info {
    bufferstate*           _bufstate_raw_ptr;
    action_t               _acquire_action;
    bool                   _syncs_on_task_finish;
    buffer_as_texture_info _tex_info;
    bool                   _uses_preacquired_arena;

    buffer_info() :
      _bufstate_raw_ptr(nullptr),
      _acquire_action(action_t::acquire_r),
      _syncs_on_task_finish(true),
      _tex_info(),
      _uses_preacquired_arena(false)
    {}

    buffer_info(bufferstate*            bufstate_raw_ptr,
                action_t                acquire_action,
                bool                    syncs_on_task_finish,
                buffer_as_texture_info  tex_info = buffer_as_texture_info()) :
      _bufstate_raw_ptr(bufstate_raw_ptr),
      _acquire_action(acquire_action),
      _syncs_on_task_finish(syncs_on_task_finish),
      _tex_info(tex_info),
      _uses_preacquired_arena(false)
    {}

    std::string to_string() const {
      return symphony::internal::strprintf("(bs=%p %s %s)",
                                           _bufstate_raw_ptr,
                                           internal::to_string(_acquire_action).c_str(),
                                           (_syncs_on_task_finish ? "sync" : "nosync"));
    }
  };

  using buffers_array = typename std::conditional<FixedSize,
                                                  std::array<buffer_info, MaxNumBuffers>,
                                                  std::vector<buffer_info>>::type;

  using multidevice_arena = std::array<arena*, MultiDeviceCount>;

  using arenas_array  = typename std::conditional<FixedSize,
                                                  std::array<multidevice_arena, MaxNumBuffers>,
                                                  std::vector<multidevice_arena>>::type;

  std::array<symphony::internal::executor_device, MultiDeviceCount> _multi_ed;

  buffers_array _arr_buffers;

  arenas_array _acquired_arenas;

  size_t _num_buffers_added;

  acquire_status _acquire_status;

  bool _lock_buffers;

  inline
  acquire_status get_acquire_status() const { return _acquire_status; }

  void acquire_precheck_and_setup(symphony::internal::executor_device_bitset edb)
  {
    SYMPHONY_INTERNAL_ASSERT(get_acquire_status() == acquire_status::idle,
                             "acquire_buffers(): buffers are already acquired or a prior acquisition is underway");
    SYMPHONY_INTERNAL_ASSERT(edb.count() > 0, "No executor device provided");
    SYMPHONY_INTERNAL_ASSERT(edb.count() <= MultiDeviceCount,
                             "buffer_acquire_set can hold only %zu executor devices, request with %zu devices, edb=%s",
                             MultiDeviceCount,
                             edb.count(),
                             internal::to_string(edb).c_str());
    SYMPHONY_INTERNAL_ASSERT(!edb.has(executor_device::unspecified),
                             "unspecified device type cannot be used: edb=%s",
                             internal::to_string(edb).c_str());

    size_t ed_count = 0;
    edb.for_each([&](executor_device ed)
    {
      SYMPHONY_INTERNAL_ASSERT(ed == executor_device::cpu ||
                               ed == executor_device::gpucl ||
                               ed == executor_device::gpugl ||
                               ed == executor_device::hexagon,
                               "ed = %zu is not an executor device type for a task.",
                               static_cast<size_t>(ed));
      _multi_ed[ed_count++] = ed;
    });
    for(; ed_count < MultiDeviceCount; ed_count++)
      _multi_ed[ed_count] = executor_device::unspecified;

    std::sort(_arr_buffers.begin(),
              _arr_buffers.begin()+_num_buffers_added,
              [](buffer_info const& v1, buffer_info const& v2)
              { return v1._bufstate_raw_ptr < v2._bufstate_raw_ptr; });

    resize_arenas_array<arenas_array>::resize(_arr_buffers.size(), _acquired_arenas);
  }

  void assign_preacquired_arenas_to_devices_for_single_buffer(bufferpolicy* bp,
                                                              symphony::internal::executor_device_bitset const& edb_to_use,
                                                              bufferstate* bs,
                                                              preacquired_arenas_base const* p_preacquired_arenas,
                                                              bufferpolicy::conflict_info& conflict)
  {
#ifdef SYMPHONY_CHECK_INTERNAL

    for(size_t idev = 0; idev < _multi_ed.size(); idev++) {
      auto ed = _multi_ed[idev];
      auto ied = static_cast<size_t>(ed);
      if(ed == executor_device::unspecified)
        break;
      SYMPHONY_INTERNAL_ASSERT(conflict._acquired_arena_per_device[ied] == nullptr,
                               "Expected no arenas to have been assigned per device as yet. bufstate=%p ed=%zu",
                               bs,
                               static_cast<size_t>(ed));
    }
#endif

    preacquired_arena_finder paf = p_preacquired_arenas->find_preacquired_arena(bs);
    while(paf.found()) {
      auto preacquired_arena = paf.entity();

      for(size_t idev = 0; idev < _multi_ed.size(); idev++) {
        auto ed = _multi_ed[idev];
        SYMPHONY_INTERNAL_ASSERT(preacquired_arena != nullptr, "preacquired_arena cannot be nullptr.");
        if(ed == executor_device::unspecified)
          break;
        if(bp->get_arena_type_accessed_by_device(ed) == arena_state_manip::get_type(preacquired_arena)) {
          auto ied = static_cast<size_t>(ed);

          if(edb_to_use.has(ed))
            conflict._acquired_arena_per_device[ied] = preacquired_arena;
        }
      }

      paf = p_preacquired_arenas->find_preacquired_arena(bs, paf.index()+1);
    }

#ifdef SYMPHONY_CHECK_INTERNAL

    edb_to_use.for_each([&](executor_device ed)
    {
      auto ied = static_cast<size_t>(ed);
      SYMPHONY_INTERNAL_ASSERT(conflict._acquired_arena_per_device[ied] != nullptr &&
                               conflict._acquired_arena_per_device[ied] != reinterpret_cast<arena*>(s_flag_tentative_acquire) &&
                               conflict._acquired_arena_per_device[ied] != reinterpret_cast<arena*>(s_flag_fake_arena),
                               "Expected a preacquired arena for every device in edb_to_use=%s for bufstate=%p",
                               internal::to_string(edb_to_use).c_str(),
                               bs);
    });
#endif
  }

  void acquire_single_buffer_or_find_conflict(bufferpolicy* bp,
                                              void const* requestor,
                                              symphony::internal::executor_device_bitset const& specialized_edb,
                                              bufferstate* bs,
                                              buffer_as_texture_info& tex_info,
                                              action_t const ac,
                                              size_t const pass,
                                              bool const setup_task_deps_on_conflict,
                                              bufferpolicy::conflict_info& conflict)
  {
    bool retry_buffer_acquire = false;
    do {
      conflict = bp->request_acquire_action(bs,
                                            requestor,
                                            specialized_edb,
                                            ac,
                                            (pass == 1 ? bufferpolicy::acquire_scope::tentative :
                                                         bufferpolicy::acquire_scope::confirm),
                                            tex_info,
                                            _lock_buffers);
      if(conflict._no_conflict_found == false) {

        SYMPHONY_INTERNAL_ASSERT(pass == 1, "buffer conflicts are expected to be found only in pass 1");

        if(!setup_task_deps_on_conflict) {
          SYMPHONY_INTERNAL_ASSERT(conflict._no_conflict_found == false, "Violated invariant");
          return;
        }

        while(conflict._no_conflict_found == false && conflict._conflicting_requestor == nullptr) {
          conflict = bp->request_acquire_action(bs,
                                                requestor,
                                                specialized_edb,
                                                ac,
                                                bufferpolicy::acquire_scope::tentative,
                                                tex_info);
        }

        if(conflict._no_conflict_found == false) {

          SYMPHONY_INTERNAL_ASSERT(conflict._conflicting_requestor != nullptr,
                               "Should have remained in while loop!");

          auto conflicting_task = static_cast<task*>(const_cast<void*>(conflict._conflicting_requestor));
          SYMPHONY_INTERNAL_ASSERT(conflicting_task != nullptr,
                               "Conflict was confirmed, but conflicting requestor was not identified");
          auto current_task = static_cast<task*>(const_cast<void*>(requestor));
          SYMPHONY_INTERNAL_ASSERT(current_task != nullptr,
                               "Requestor task not specified");
          SYMPHONY_INTERNAL_ASSERT(conflicting_task != current_task,
                               "Buffer already held by the same task");

          if (conflicting_task->add_dynamic_control_dependency(current_task)) {

            SYMPHONY_INTERNAL_ASSERT(conflict._no_conflict_found == false, "Violated invariant");
            return;
          }
          else {

            retry_buffer_acquire = true;
          }
        }

      }
    } while (retry_buffer_acquire);

    SYMPHONY_INTERNAL_ASSERT(conflict._no_conflict_found == true, "Violated invariant");
    return;
  }

  bool attempt_acquire_pass(size_t const pass,
                            bufferpolicy* bp,
                            void const* requestor,
                            symphony::internal::executor_device_bitset const& edb,
                            bool const setup_task_deps_on_conflict,
                            preacquired_arenas_base const* p_preacquired_arenas,
                            override_device_sets_base const* p_override_device_sets)
  {
    size_t index = 0;
    while(index < _num_buffers_added) {
      auto bs       = _arr_buffers[index]._bufstate_raw_ptr;
      auto ac       = _arr_buffers[index]._acquire_action;
      bool sotf     = _arr_buffers[index]._syncs_on_task_finish;
      auto tex_info = _arr_buffers[index]._tex_info;

      auto edb_to_use = edb;
      if(p_override_device_sets != nullptr) {
        override_device_set_finder odf = p_override_device_sets->find_override_device_set(bs);
        if(odf.found()) {
          edb_to_use = odf.entity();

#ifdef SYMPHONY_CHECK_INTERNAL

          edb_to_use.for_each([&](executor_device ed)
          {
            SYMPHONY_INTERNAL_ASSERT(edb.has(ed),
                                     "override edb=%s must be a subset of general edb=%s, bs=%p",
                                     internal::to_string(edb_to_use).c_str(),
                                     internal::to_string(edb).c_str(),
                                     bs);
          });
#endif
        }
      }

      while(index+1 < _num_buffers_added && bs == _arr_buffers[index+1]._bufstate_raw_ptr) {

        for(auto& a : _acquired_arenas[index])
          a = nullptr;

        auto next_ac   = _arr_buffers[index+1]._acquire_action;
        bool next_sotf = _arr_buffers[index+1]._syncs_on_task_finish;
        if(ac != next_ac) {

          ac = action_t::acquire_rw;

        }
        sotf |= next_sotf;
        index++;
      }

      _arr_buffers[index]._syncs_on_task_finish = sotf;

      bufferpolicy::conflict_info conflict{true, nullptr};

      bool any_preacquired_arena_for_buffer = (p_preacquired_arenas != nullptr &&
                                               p_preacquired_arenas->find_preacquired_arena(bs).found());
      if(any_preacquired_arena_for_buffer)
      {

        if(pass == 2) {
          assign_preacquired_arenas_to_devices_for_single_buffer(bp,
                                                                 edb_to_use,
                                                                 bs,
                                                                 p_preacquired_arenas,
                                                                 conflict);
          SYMPHONY_INTERNAL_ASSERT(conflict._no_conflict_found == true,
                                   "No conflicts expected for a buffer with preacquired arenas. bufstate=%p",
                                   bs);
        }
      }
      else {

        executor_device_bitset specialized_edb;
        edb_to_use.for_each([&](executor_device ed)
        {
          specialized_edb.add( (ed == executor_device::gpucl && tex_info.get_used_as_texture()) ?
                               executor_device::gputexture : ed );
        });

        acquire_single_buffer_or_find_conflict(bp,
                                               requestor,
                                               specialized_edb,
                                               bs,
                                               tex_info,
                                               ac,
                                               pass,
                                               setup_task_deps_on_conflict,
                                               conflict);
      }
      _arr_buffers[index]._uses_preacquired_arena = any_preacquired_arena_for_buffer;

      if(conflict._no_conflict_found == false)
        return false;

      if(pass == 2) {

        for(size_t idev = 0; idev < _multi_ed.size(); idev++) {
          auto ed = _multi_ed[idev];
          if(ed == executor_device::unspecified)
            break;
          auto ied = static_cast<size_t>(ed);
          if(!edb_to_use.has(ed) &&
             (conflict._acquired_arena_per_device[ied] == nullptr ||
              conflict._acquired_arena_per_device[ied] == reinterpret_cast<arena*>(s_flag_tentative_acquire)))
          {
            conflict._acquired_arena_per_device[ied] = reinterpret_cast<arena*>(s_flag_fake_arena);
          }
        }
      }

      for(size_t idev=0; idev<MultiDeviceCount; idev++) {
        auto ed = _multi_ed[idev];
        SYMPHONY_INTERNAL_ASSERT(idev > 0 || ed != executor_device::unspecified,
                                 "Expected at least one valid requested executor device");
        if(ed == executor_device::unspecified)
          break;
        auto specialized_ed = (ed == executor_device::gpucl && tex_info.get_used_as_texture()) ?
                               executor_device::gputexture : ed;
        if(pass == 1) {
          SYMPHONY_INTERNAL_ASSERT(conflict._acquired_arena_per_device[static_cast<size_t>(specialized_ed)] == nullptr,
            "Pass 1 should not have acquired_arena (for ed=%zu) on tentative acquire of bufstate=%p",
            static_cast<size_t>(specialized_ed),
            bs);
          _acquired_arenas[index][idev] = reinterpret_cast<arena*>(s_flag_tentative_acquire);
        }
        else if(pass == 2) {

          SYMPHONY_INTERNAL_ASSERT(conflict._acquired_arena_per_device[static_cast<size_t>(specialized_ed)] != nullptr,
            "Pass 2 should have found valid acquired_arena (for ed=%zu) on confirmed acquire of bufstate=%p",
            static_cast<size_t>(specialized_ed),
            bs);
          _acquired_arenas[index][idev] = conflict._acquired_arena_per_device[static_cast<size_t>(specialized_ed)];
        }
        else {
          SYMPHONY_UNREACHABLE("There should only be Pass 1 and Pass 2. Found pass=%zu", pass);
        }
      }

      index++;
    }

    if(pass == 1) {

      _acquire_status = acquire_status::tentatively_acquired;;
    }
    else if(pass == 2) {

      _acquire_status = acquire_status::fully_acquired;
    }
    else {
      SYMPHONY_UNREACHABLE("Invalid pass = %zu", pass);
    }

    return true;
  }

public:
  buffer_acquire_set() :
    _multi_ed(),
    _arr_buffers(),
    _acquired_arenas(),
    _num_buffers_added(0),
    _acquire_status(acquire_status::idle),
    _lock_buffers(true)
  {}

  void enable_non_locking_buffer_acquire() {
    _lock_buffers = false;
  }

  template<typename BufferPtr>
  void add(BufferPtr& b,
           action_t acquire_action,
           buffer_as_texture_info tex_info = buffer_as_texture_info())
  {
    static_assert( is_api20_buffer_ptr<BufferPtr>::value, "must be a symphony::buffer_ptr" );
    if(b == nullptr) {
      return;
    }

    auto bufstate = buffer_accessor::get_bufstate(reinterpret_cast<buffer_ptr_base&>(b));
    auto bufstate_raw_ptr = ::symphony::internal::c_ptr(bufstate);
    SYMPHONY_INTERNAL_ASSERT(bufstate_raw_ptr != nullptr, "Non-null buffer_ptr contains a null bufferstate");

    checked_addition_of_buffer_entry<buffers_array>::add(_arr_buffers,
                                                         _num_buffers_added,
                                                         buffer_info{ bufstate_raw_ptr,
                                                                      acquire_action,
                                                                      b.syncs_on_task_finish(),
                                                                      tex_info });
  }

  size_t get_num_buffers_added() const { return _num_buffers_added; }

  inline
  bool acquired() const { return (get_acquire_status() == acquire_status::fully_acquired); }

  void acquire_buffers(void const* requestor,
                       symphony::internal::executor_device_bitset edb,
                       const bool setup_task_deps_on_conflict = false,
                       preacquired_arenas_base const* p_preacquired_arenas = nullptr,
                       override_device_sets_base const* p_override_device_sets = nullptr)
  {
    acquire_precheck_and_setup(edb);

    auto bp = get_current_bufferpolicy();

    for(size_t pass = 1; pass <= 2; pass++) {
      bool no_conflict = attempt_acquire_pass(pass,
                                              bp,
                                              requestor,
                                              edb,
                                              setup_task_deps_on_conflict,
                                              p_preacquired_arenas,
                                              p_override_device_sets);
      SYMPHONY_INTERNAL_ASSERT(pass == 1 || no_conflict,
                               "Pass 2 is not expected to encounter conflicts.");
      if(!no_conflict) {

        release_buffers(requestor);
        return;
      }
    }

    log::fire_event<log::events::buffer_set_acquired>();
  }

  void blocking_acquire_buffers(void const* requestor,
                                symphony::internal::executor_device_bitset edb,
                                preacquired_arenas_base const* p_preacquired_arenas = nullptr,
                                override_device_sets_base const* p_override_device_sets = nullptr)
  {
    size_t spin_count = 10;
    while(!acquired()) {
      SYMPHONY_INTERNAL_ASSERT(get_acquire_status() != acquire_status::tentatively_acquired,
                               "blocking_acquire_buffers(): at this granularity there should be no tentative acquire. bas=%p",
                               this);
      if(spin_count > 0)
        spin_count--;
      else
        usleep(1);

      acquire_buffers(requestor, edb, false, p_preacquired_arenas, p_override_device_sets);
    }
  }

  void release_buffers(void const* requestor)
  {
    SYMPHONY_INTERNAL_ASSERT(_multi_ed[0] != executor_device::unspecified,
                             "There must be at least one valid executor device to acquire buffers");

    auto bp = get_current_bufferpolicy();

    for(size_t i=0; i<_num_buffers_added; i++) {

      if(_acquired_arenas[i][0] == nullptr)
        continue;

      auto bufstate_raw_ptr       = _arr_buffers[i]._bufstate_raw_ptr;
      bool syncs_on_task_finish   = _arr_buffers[i]._syncs_on_task_finish;
      bool uses_preacquired_arena = _arr_buffers[i]._uses_preacquired_arena;

      std::unique_lock<std::mutex> lock(bufstate_raw_ptr->access_mutex(), std::defer_lock);
      if(_lock_buffers && (!uses_preacquired_arena || get_acquire_status() == acquire_status::fully_acquired))
        lock.lock();

      if(!uses_preacquired_arena) {
        bp->release_action(bufstate_raw_ptr,
                           requestor,
                           false);
      }

      if(get_acquire_status() == acquire_status::fully_acquired) {

        if(bufstate_raw_ptr->get_any_confirmed_acquire_requestor() == nullptr && syncs_on_task_finish == true)
        {
          SYMPHONY_DLOG("Syncing symphony::buffer_ptr: bufstate_raw_ptr = %p on release by requestor=%p",
                        bufstate_raw_ptr,
                        requestor);

          bufstate_raw_ptr->sync_on_task_finish();
        }
        SYMPHONY_DLOG("bufstate=%p %s",
                      bufstate_raw_ptr,
                      bufstate_raw_ptr->to_string().c_str());

      }

      for(auto& a : _acquired_arenas[i])
        a = nullptr;
    }

    _acquire_status = acquire_status::idle;

  }

  template<typename BufferPtr>
  arena* find_acquired_arena(BufferPtr& b,
                             symphony::internal::executor_device ed = symphony::internal::executor_device::unspecified)
  {
    SYMPHONY_INTERNAL_ASSERT(get_acquire_status() == acquire_status::fully_acquired,
                             "buffer_acquire_set::find_acquired_arena(): Attempting to find"
                             "arena when buffers not acquired");

    static_assert( is_api20_buffer_ptr<BufferPtr>::value, "must be a symphony::buffer_ptr" );
    if(b == nullptr) {
      return nullptr;
    }

    auto bufstate = buffer_accessor::get_bufstate(
                                            reinterpret_cast<symphony::internal::buffer_ptr_base&>(b));
    auto bufstate_raw_ptr = ::symphony::internal::c_ptr(bufstate);

    if(MultiDeviceCount == 1 && ed == symphony::internal::executor_device::unspecified)
      ed = _multi_ed[0];

    SYMPHONY_INTERNAL_ASSERT(ed != symphony::internal::executor_device::unspecified, "Invalid executor device");

    size_t idev = 0;
    for( ; idev<MultiDeviceCount; idev++) {
      if(_multi_ed[idev] == ed)
        break;
    }
    SYMPHONY_API_ASSERT(idev < MultiDeviceCount,
                        "executor device ed=%zu was not used to acquire buffers: failure on bufstate=%p, multi_ed=%s",
                        static_cast<size_t>(ed),
                        bufstate_raw_ptr,
                        internal::to_string(_multi_ed).c_str());
    SYMPHONY_INTERNAL_ASSERT(_multi_ed[idev] == ed, "Impossible! Linear search is kaput.");

    for(size_t i=0; i<_num_buffers_added; i++) {
      if(_arr_buffers[i]._bufstate_raw_ptr == bufstate_raw_ptr && _acquired_arenas[i][idev] != nullptr) {
        SYMPHONY_INTERNAL_ASSERT(_acquired_arenas[i][idev] != reinterpret_cast<arena*>(s_flag_tentative_acquire),
                                 "Only tentative arena found: bs=%p ed=%s",
                                 _arr_buffers[i]._bufstate_raw_ptr,
                                 internal::to_string(ed).c_str());

        if(_acquired_arenas[i][idev] == reinterpret_cast<arena*>(s_flag_fake_arena))
          return nullptr;

        return _acquired_arenas[i][idev];
      }
    }
    return nullptr;
  }

  std::string to_string() const {
    std::string s = symphony::internal::strprintf("bas %p %s num_buffers_added=%zu acquired=%c locking=%c\n",
                                                  this,
                                                  (FixedSize ? "FixedSize" : "DynSize"),
                                                  _num_buffers_added,
                                                  (acquired() ? 'Y' : 'N'),
                                                  (_lock_buffers ? 'Y' : 'N'));
    s.append("  buffers = [");
    for(auto const& buf_info : _arr_buffers)
      s.append(buf_info.to_string() + " ");
    s.append("]\n");

    if(acquired()) {
      s.append("  acquired arenas :\n");
      for(size_t idev = 0; idev < _multi_ed.size(); idev++) {
        auto ed = _multi_ed[idev];
        if(ed == executor_device::unspecified)
          break;

        s.append("    " + internal::to_string(ed) + " : ");
        for(size_t i=0; i < _acquired_arenas.size(); i++) {
          SYMPHONY_INTERNAL_ASSERT(idev < _acquired_arenas[i].size(), "idev=%zu must be < _acquired_arenas[%zu].size()=%zu",
                                   idev,
                                   i,
                                   _acquired_arenas[i].size());
          s.append(symphony::internal::strprintf("%p ", _acquired_arenas[i][idev]));
        }
        s.append("\n");
      }
    }

    return s;
  }
};

template<typename BufferWithDir>
struct strip_buffer_dir  {
 using type = BufferWithDir;
};

template<typename T>
struct strip_buffer_dir<symphony::out<symphony::buffer_ptr<T>>>  {
 using type = symphony::buffer_ptr<T>;
};

template<typename T>
struct strip_buffer_dir<symphony::in<symphony::buffer_ptr<T>>>  {
 using type = symphony::buffer_ptr<T>;
};

template<typename T>
struct strip_buffer_dir<symphony::inout<symphony::buffer_ptr<T>>>  {
 using type = symphony::buffer_ptr<T>;
};

};
};
