// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/buffer/executordevice.hh>
#include <symphony/internal/device/gpuopencl.hh>
#include <symphony/internal/device/gpuopengl.hh>

namespace symphony {
namespace graphics {
namespace internal {

#ifdef SYMPHONY_HAVE_OPENCL

class base_texture_cl;
#endif

};
};
};

namespace symphony {
namespace internal {

enum arena_t : size_t {
  MAINMEM_ARENA = 0,
#ifdef SYMPHONY_HAVE_OPENCL
  CL_ARENA      = 1,
#endif
#ifdef SYMPHONY_HAVE_OPENCL_2_0
  CL2_ARENA     = 2,
#endif
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
  ION_ARENA     = 3,
#endif
#ifdef SYMPHONY_HAVE_GLES
  GL_ARENA      = 4,
#endif
#ifdef SYMPHONY_HAVE_OPENCL
  TEXTURE_ARENA = 5,
#endif
  NO_ARENA      = 6
};

constexpr size_t NUM_ARENA_TYPES = arena_t::NO_ARENA;

enum arena_alloc_t{
  UNALLOCATED,
  INTERNAL,
  EXTERNAL,
  BOUND
};

class arena;
class mainmem_arena;
#ifdef SYMPHONY_HAVE_OPENCL
class cl_arena;
#endif
#ifdef SYMPHONY_HAVE_OPENCL_2_0
class cl2_arena;
#endif
#ifdef SYMPHONY_HAVE_QTI_HEXAGON
class ion_arena;
#endif
#ifdef SYMPHONY_HAVE_GLES
class gl_arena;
#endif
#ifdef SYMPHONY_HAVE_OPENCL
class texture_arena;
#endif

bool can_copy (arena* src, arena* dest);
void copy_arenas(arena* src, arena* dest);

struct arena_state_manip {

  static void delete_arena(arena* a);

  static arena_t get_type(arena* a);

  static arena_alloc_t get_alloc_type(arena* a);

  static void invalidate(arena* a);

  static arena* get_bound_to_arena(arena* a);

  static void ref(arena* a);

  static void unref(arena* a);

  static size_t get_ref_count(arena* a);
};

struct arena_storage_accessor {
  static void* access_mainmem_arena_for_cputask(arena* acquired_arena);

#ifdef SYMPHONY_HAVE_OPENCL
#ifdef SYMPHONY_CL_TO_CL2
  static void* access_cl2_arena_for_gputask(arena* acquired_arena);
#else
  static cl::Buffer& access_cl_arena_for_gputask(arena* acquired_arena);
#endif
#endif

#ifdef SYMPHONY_HAVE_GLES
  static GLuint access_gl_arena_for_gputask(arena* acquired_arena);
#endif

#ifdef SYMPHONY_HAVE_OPENCL
  static symphony::graphics::internal::base_texture_cl* access_texture_arena_for_gputask(arena* acquired_arena);
#endif

#ifdef SYMPHONY_HAVE_QTI_HEXAGON
  static void* access_ion_arena_for_hexagontask(arena* acquired_arena);
#endif

#ifdef SYMPHONY_HAVE_OPENCL
  static bool texture_arena_has_format(arena* a);
#endif

  static void* get_ptr(arena* a);
};

};
};
