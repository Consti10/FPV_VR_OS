// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstddef>

#include <symphony/internal/compat/compilercompat.h>
#ifdef SYMPHONY_HAVE_OPENCL
#include <symphony/internal/device/cldevice.hh>
#include <symphony/internal/device/gpuopencl.hh>
#endif
#ifdef SYMPHONY_HAVE_GLES
#include <symphony/internal/device/gpuopengl.hh>
#endif
#include <symphony/internal/memalloc/alignedmalloc.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/scopeguard.hh>

namespace symphony{
namespace internal{

#ifdef SYMPHONY_HAVE_OPENCL
extern std::vector<legacy::device_ptr> *s_dev_ptrs;
#endif

enum class memregion_t {
  none,
  main,
  cl2svm,
  ion,
  glbuffer
};

class internal_memregion{
public:
  internal_memregion() : _num_bytes(0){}

  explicit internal_memregion(size_t sz) : _num_bytes(sz) {}

  virtual ~internal_memregion(){}

  virtual memregion_t get_type() const { return memregion_t::none; }

  virtual void* get_ptr(void ) const {
    SYMPHONY_FATAL("Calling get_ptr() method of base class internal_memregion.");
    return nullptr;
  }

#ifdef SYMPHONY_HAVE_GLES
  virtual GLuint get_id() const {
    SYMPHONY_FATAL("Calling get_id() method of base class internal_memregion.");
  }
#else
  virtual int get_id() const {
    SYMPHONY_FATAL("Calling get_id() method of base class internal_memregion.");
   }
#endif

  size_t get_num_bytes() const { return _num_bytes;}

  virtual bool is_cacheable() const {return false;}

  virtual int get_fd() const {
    SYMPHONY_FATAL("Calling get_fd of base class internal_memregion.");
    return -1;
  }

protected:
  size_t _num_bytes;

  SYMPHONY_DELETE_METHOD(internal_memregion(internal_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_memregion& operator=(internal_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_memregion(internal_memregion&&));
  SYMPHONY_DELETE_METHOD(internal_memregion& operator=(internal_memregion&&));
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
class internal_main_memregion : public internal_memregion {
SYMPHONY_GCC_IGNORE_END("-Weffc++");
private:
  void* _p;
  bool _is_internally_allocated;
public:
  internal_main_memregion(size_t sz, size_t alignment) :
    internal_memregion(sz),
    _p(nullptr),
    _is_internally_allocated(true)
  {
    SYMPHONY_INTERNAL_ASSERT(sz != 0, "Error. Specified size of mem region is 0");
    _p = ::symphony::internal::symphony_aligned_malloc(alignment,sz);
  }

  internal_main_memregion(void* ptr, size_t sz) :
    internal_memregion(sz),
    _p(ptr),
    _is_internally_allocated(false)
  {

    SYMPHONY_API_ASSERT(ptr != nullptr, "Error. Expect non nullptr for externally allocated memregion");
  }

  ~internal_main_memregion()
  {

    if(_is_internally_allocated){
      SYMPHONY_INTERNAL_ASSERT(_p != nullptr, "Error. mem region pointer is nullptr");
      ::symphony::internal::symphony_aligned_free(_p);
    }
  }

  void* get_ptr(void ) const {return _p;}

  memregion_t get_type() const {return memregion_t::main;}

  SYMPHONY_DELETE_METHOD(internal_main_memregion(internal_main_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_main_memregion& operator=(internal_main_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_main_memregion(internal_main_memregion&&));
  SYMPHONY_DELETE_METHOD(internal_main_memregion& operator=(internal_main_memregion&&));
};

#ifdef SYMPHONY_HAVE_OPENCL_2_0
SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
class internal_cl2svm_memregion : public internal_memregion {
SYMPHONY_GCC_IGNORE_END("-Weffc++");
private:
  void* _ptr;
  bool _is_internally_allocated;
  cl::SVMAllocator<char, cl::SVMTraitCoarse<>> _svm_allocator;
public:
  explicit internal_cl2svm_memregion (size_t sz) :
    internal_memregion(sz),
    _ptr(nullptr),
    _is_internally_allocated(true),
    _svm_allocator(::symphony::internal::c_ptr(::symphony::internal::get_default_cldevice())->
                   get_context())
  {
    SYMPHONY_INTERNAL_ASSERT(sz != 0, "Error. Specified size of mem region is 0");
    try {
      _ptr = _svm_allocator.allocate (sz / sizeof(char));
    }
    catch (cl::Error & err) {
      SYMPHONY_FATAL("cl::SVMAllocator::allocate(%zu)->%s",
                 sz / sizeof(char),
                 ::symphony::internal::get_cl_error_string(err.err()));
    }
    catch (...) {
      SYMPHONY_FATAL("Unknown error in cl::SVMAllocator::allocate(%zu)",
                 sz / sizeof(char));
    }
  }

  internal_cl2svm_memregion(void* ptr, size_t sz) :
    internal_memregion(sz),
    _ptr(ptr),
    _is_internally_allocated(false),
    _svm_allocator()
  {
    SYMPHONY_API_ASSERT(ptr != nullptr,
                    "Error. Expect non nullptr for externally allocated memregion");
  }

  ~internal_cl2svm_memregion ()
  {
    if (_is_internally_allocated) {
      SYMPHONY_INTERNAL_ASSERT(_ptr != nullptr, "Error. svm region pointer is nullptr");
      _svm_allocator.deallocate(static_cast<char*>(_ptr), _num_bytes);
    }
  }

  void* get_ptr() const { return _ptr;}

  memregion_t get_type() const { return memregion_t::cl2svm;}

  SYMPHONY_DELETE_METHOD(internal_cl2svm_memregion(internal_cl2svm_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_cl2svm_memregion& operator=(internal_cl2svm_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_cl2svm_memregion(internal_cl2svm_memregion&&));
  SYMPHONY_DELETE_METHOD(internal_cl2svm_memregion& operator=(internal_cl2svm_memregion&&));
};
#endif

#ifdef SYMPHONY_HAVE_QTI_HEXAGON
SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
class internal_ion_memregion : public internal_memregion {
 SYMPHONY_GCC_IGNORE_END("-Weffc++");
private:
  bool _cacheable;

  void* _p;

  bool _is_internally_allocated;

  int _fd;

public:
  internal_ion_memregion(size_t sz, bool cacheable);

  internal_ion_memregion(void* ptr, size_t sz, bool cacheable);

  internal_ion_memregion(void* ptr, int fd, size_t sz, bool cacheable);

  ~internal_ion_memregion();

  void* get_ptr(void ) const {return _p;}

  memregion_t get_type() const {return memregion_t::ion;}

  bool is_cacheable() const {return _cacheable;}

  int get_fd() const {return _fd;}

  SYMPHONY_DELETE_METHOD(internal_ion_memregion(internal_ion_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_ion_memregion& operator=(internal_ion_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_ion_memregion(internal_ion_memregion&&));
  SYMPHONY_DELETE_METHOD(internal_ion_memregion& operator=(internal_ion_memregion&&));
};
#endif

#ifdef SYMPHONY_HAVE_GLES
class internal_glbuffer_memregion : public internal_memregion {
private:
  GLuint _id;
public:

  explicit internal_glbuffer_memregion(GLuint id):
    internal_memregion(),
    _id(id)
  {

    GLint old_value;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_value);

    auto guard = make_scope_guard([old_value] {
        glBindBuffer(GL_ARRAY_BUFFER, old_value);
        GLenum error = glGetError();
        SYMPHONY_API_THROW((error == GL_NO_ERROR), "glBindBuffer(0)->%x", error);
      });

    glBindBuffer(GL_ARRAY_BUFFER, _id);
    GLenum error = glGetError();

    SYMPHONY_API_THROW((error == GL_NO_ERROR), "glBindBuffer(%u)->%x", _id,
                     error);

    SYMPHONY_API_THROW((glIsBuffer(_id) == GL_TRUE), "Invalid GL buffer");

    GLint gl_buffer_size = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &gl_buffer_size);
    error = glGetError();
    SYMPHONY_API_THROW((error == GL_NO_ERROR), "glGetBufferParameteriv()->%d", error);

    _num_bytes = gl_buffer_size;
    SYMPHONY_DLOG("GL buffer object size: %zu", _num_bytes);
    SYMPHONY_API_THROW(_num_bytes != 0, "GL buffer size can't be 0");
  }

  GLuint get_id() const {return _id;}

  memregion_t get_type() const {return memregion_t::glbuffer;}

  SYMPHONY_DELETE_METHOD(internal_glbuffer_memregion(internal_glbuffer_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_glbuffer_memregion& operator=(internal_glbuffer_memregion const&));
  SYMPHONY_DELETE_METHOD(internal_glbuffer_memregion(internal_glbuffer_memregion&&));
  SYMPHONY_DELETE_METHOD(internal_glbuffer_memregion& operator=(internal_glbuffer_memregion&&));
};
#endif

};
};
