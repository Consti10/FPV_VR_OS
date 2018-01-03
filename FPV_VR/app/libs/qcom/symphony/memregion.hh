// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/buffer/memregion.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>

namespace symphony {
namespace internal {

class memregion_base_accessor;
};

class memregion {

  friend
  class ::symphony::internal::memregion_base_accessor;

protected:
  explicit memregion(internal::internal_memregion* int_mr) : _int_mr(int_mr){}

  internal::internal_memregion* _int_mr;

public:

  ~memregion(){
    SYMPHONY_INTERNAL_ASSERT(_int_mr != nullptr, "Error. Internal memregion is null");
    delete _int_mr;
  }

  size_t get_num_bytes() const { return _int_mr->get_num_bytes(); }

  SYMPHONY_DELETE_METHOD(memregion());
  SYMPHONY_DELETE_METHOD(memregion(memregion const&));
  SYMPHONY_DELETE_METHOD(memregion& operator= (memregion const&));
  SYMPHONY_DELETE_METHOD(memregion (memregion &&));
  SYMPHONY_DELETE_METHOD(memregion& operator= (memregion &&));

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

class main_memregion : public memregion{
public:

  static constexpr size_t s_default_alignment = 4096;

  explicit main_memregion(size_t sz, size_t alignment=s_default_alignment):
    memregion(new internal::internal_main_memregion(sz, alignment)){}

  main_memregion(void* ptr, size_t sz):
    memregion(new internal::internal_main_memregion(ptr, sz)){}

  SYMPHONY_DELETE_METHOD(main_memregion());
  SYMPHONY_DELETE_METHOD(main_memregion(main_memregion const&));
  SYMPHONY_DELETE_METHOD(main_memregion& operator= (main_memregion const&));
  SYMPHONY_DELETE_METHOD(main_memregion (main_memregion &&));
  SYMPHONY_DELETE_METHOD(main_memregion& operator= (main_memregion &&));

  void* get_ptr() const {
    return _int_mr->get_ptr();
  }

};

#ifdef SYMPHONY_HAVE_OPENCL_2_0

class cl2svm_memregion : public memregion{
public:

  explicit cl2svm_memregion(size_t sz):
    memregion(new internal::internal_cl2svm_memregion(sz)){}

  cl2svm_memregion(void* ptr, size_t sz):
    memregion(new internal::internal_cl2svm_memregion(ptr, sz)){}

  SYMPHONY_DELETE_METHOD(cl2svm_memregion());
  SYMPHONY_DELETE_METHOD(cl2svm_memregion(cl2svm_memregion const&));
  SYMPHONY_DELETE_METHOD(cl2svm_memregion& operator= (cl2svm_memregion const&));
  SYMPHONY_DELETE_METHOD(cl2svm_memregion (cl2svm_memregion &&));
  SYMPHONY_DELETE_METHOD(cl2svm_memregion& operator= (cl2svm_memregion &&));

  void* get_ptr() const {
    return _int_mr->get_ptr();
  }
};
#endif

#ifdef SYMPHONY_HAVE_QTI_HEXAGON

class ion_memregion : public memregion{

public:

  explicit ion_memregion(size_t sz, bool cacheable=true):
    memregion(new internal::internal_ion_memregion(sz, cacheable)){}

  ion_memregion(void* ptr, size_t sz, bool cacheable):
    memregion(new internal::internal_ion_memregion(ptr, sz, cacheable)){}

    ion_memregion(void* ptr, int fd, size_t sz, bool cacheable):
      memregion(new internal::internal_ion_memregion(ptr, fd, sz, cacheable)){}

  SYMPHONY_DELETE_METHOD(ion_memregion());
  SYMPHONY_DELETE_METHOD(ion_memregion(ion_memregion const&));
  SYMPHONY_DELETE_METHOD(ion_memregion& operator= (ion_memregion const&));
  SYMPHONY_DELETE_METHOD(ion_memregion (ion_memregion &&));
  SYMPHONY_DELETE_METHOD(ion_memregion& operator= (ion_memregion &&));

  void* get_ptr() const {return _int_mr->get_ptr();}

  int get_fd() const {return _int_mr->get_fd();}

  bool is_cacheable() const {return _int_mr->is_cacheable();}

};
#endif

#ifdef SYMPHONY_HAVE_GLES

class glbuffer_memregion : public memregion{
public:

  explicit glbuffer_memregion(GLuint id):
    memregion(new internal::internal_glbuffer_memregion(id)){}

  SYMPHONY_DELETE_METHOD(glbuffer_memregion());
  SYMPHONY_DELETE_METHOD(glbuffer_memregion(glbuffer_memregion const&));
  SYMPHONY_DELETE_METHOD(glbuffer_memregion& operator= (glbuffer_memregion const&));
  SYMPHONY_DELETE_METHOD(glbuffer_memregion (glbuffer_memregion &&));
  SYMPHONY_DELETE_METHOD(glbuffer_memregion& operator= (glbuffer_memregion &&));

  GLuint get_id() const {return _int_mr->get_id();}
};
#endif

SYMPHONY_GCC_IGNORE_END("-Weffc++");

};
