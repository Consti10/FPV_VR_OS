// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <stdexcept>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/patterns/pipeline/pipelinebuffers.hh>

namespace symphony {
namespace internal {

  template<typename... UserData> class pipeline_skeleton;

  class pipeline_stage_skeleton_base;

  template<typename F, typename... UserData>
  class pipeline_cpu_stage_skeleton;
  template<typename BeforeBody, typename GK, typename AfterBody, typename... UserData>
  class pipeline_gpu_stage_skeleton;

  class pipeline_stage_instance;
  class pipeline_instance_base;
  template<typename... UserData> class pipeline_instance;

  template <typename F, size_t Arity, typename RT>
  class cpu_stage_function_arity_dispatch;
  template <typename F, size_t Arity, typename RT>
  class gpu_stage_function_arity_dispatch;

};
};

namespace symphony{

typedef enum serial_stage_type{
  in_order = 0
} serial_stage_type;

class serial_stage {

  serial_stage_type _type;

public:

  explicit serial_stage(serial_stage_type t = serial_stage_type::in_order):_type(t) {}

  serial_stage_type get_type() const {return _type;}

   serial_stage(serial_stage const& other) :
    _type(other._type) {
  };

  explicit serial_stage(serial_stage&& other) :
    _type(other._type) {
  };

  serial_stage& operator=(serial_stage const& other) {
    _type = other._type;
    return *this;
  }

  SYMPHONY_DELETE_METHOD(serial_stage& operator=(serial_stage&& other));

  friend class symphony::internal::pipeline_stage_skeleton_base;

};

class parallel_stage {

  size_t _degree_of_concurrency;

public:

  explicit parallel_stage(size_t doc):_degree_of_concurrency(doc) {
    SYMPHONY_API_ASSERT(doc > 0, "degree of concurrency should be larger than 0.");
  }

  size_t get_degree_of_concurrency() const {return _degree_of_concurrency;}

   parallel_stage(parallel_stage const& other) :
    _degree_of_concurrency(other._degree_of_concurrency) {}

  parallel_stage& operator=(parallel_stage const& other) {
    _degree_of_concurrency = other._degree_of_concurrency;
    return *this;
  }

  SYMPHONY_DELETE_METHOD(parallel_stage& operator=(parallel_stage&& other));

  friend class symphony::internal::pipeline_stage_skeleton_base;

};

class iteration_lag{

  size_t _iter_lag;

public:

  explicit iteration_lag(size_t lag):_iter_lag(lag) {}

  size_t get_iter_lag() const {return _iter_lag;}

   iteration_lag(iteration_lag const& other) :
    _iter_lag(other._iter_lag) {}

  explicit iteration_lag(iteration_lag&& other) :
  _iter_lag(other._iter_lag) {}

  iteration_lag& operator=(iteration_lag const& other) {
    _iter_lag = other._iter_lag;
    return *this;
  }

  SYMPHONY_DELETE_METHOD(iteration_lag& operator=(iteration_lag && other));

  template<typename F, typename... UserData>
  friend class symphony::internal::pipeline_cpu_stage_skeleton;

  template<typename BeforeBody, typename GK, typename AfterBody, typename... UserData>
  friend class symphony::internal::pipeline_gpu_stage_skeleton;

};

class iteration_rate{

  size_t _iter_rate_pred;

  size_t _iter_rate_curr;

public:

  iteration_rate(size_t p, size_t c):_iter_rate_pred(p), _iter_rate_curr(c) {}

  size_t get_iter_rate_pred() const {return _iter_rate_pred;};

  size_t get_iter_rate_curr() const {return _iter_rate_curr;};

   iteration_rate(iteration_rate const& other) :
   _iter_rate_pred(other._iter_rate_pred),
   _iter_rate_curr(other._iter_rate_curr) {
  }

  explicit iteration_rate(iteration_rate&& other) :
   _iter_rate_pred(other._iter_rate_pred),
   _iter_rate_curr(other._iter_rate_curr) {
  }

  iteration_rate& operator=(iteration_rate const& other) {
    _iter_rate_pred = other._iter_rate_pred;
    _iter_rate_curr = other._iter_rate_curr;
    return *this;
  }

  SYMPHONY_DELETE_METHOD(iteration_rate& operator=(iteration_rate&& other));

  template<typename F, typename... UD>
  friend class symphony::internal::pipeline_cpu_stage_skeleton;

  template<typename BeforeBody, typename GK, typename AfterBody, typename... UD>
  friend class symphony::internal::pipeline_gpu_stage_skeleton;

};

class sliding_window_size{

  size_t _size;

public:

  explicit sliding_window_size(size_t size):_size(size) {}

  size_t get_size() const {return _size;}

   sliding_window_size(sliding_window_size const& other):
  _size(other._size) {}

  explicit sliding_window_size(sliding_window_size&& other):
  _size(other._size) {}

  sliding_window_size& operator=(sliding_window_size const& other) {
    _size = other._size;
    return *this;
  }

  SYMPHONY_DELETE_METHOD(sliding_window_size& operator=(sliding_window_size&& other));

  template<typename F, typename... UserData>
  friend class symphony::internal::pipeline_cpu_stage_skeleton;

  template<typename BeforeBody, typename GK, typename AfterBody, typename... UserData>
  friend class symphony::internal::pipeline_gpu_stage_skeleton;

  template<typename... UserData> friend class symphony::internal::pipeline_skeleton;

};

class stage_input_base{
protected:

  stage_input_base() {}

public:

  virtual ~stage_input_base() {};

  SYMPHONY_DELETE_METHOD(stage_input_base(stage_input_base const& other));
  SYMPHONY_DELETE_METHOD(stage_input_base(stage_input_base&& other));
  SYMPHONY_DELETE_METHOD(stage_input_base& operator=(stage_input_base const& other));
  SYMPHONY_DELETE_METHOD(stage_input_base& operator=(stage_input_base&& other));
};

template<typename InputType>
class stage_input: public stage_input_base{
private:

  symphony::internal::stagebuffer* _buffer;

  size_t _first;

  size_t _offset;

  size_t _size;

  internal::pipeline_launch_type _launch_type;

  stage_input(symphony::internal::stagebuffer* buffer,
              size_t offset,
              size_t input_size,
              size_t first,
              internal::pipeline_launch_type launch_type):
   _buffer(buffer),
   _first(first),
   _offset(offset),
   _size(input_size),
   _launch_type(launch_type) {}

public:

  typedef InputType input_type;

  virtual ~stage_input() {};

  InputType& operator[](size_t i) {
    SYMPHONY_API_THROW_CUSTOM(i < _size,
                          std::out_of_range,
                          "Out of range access to stage_input.");
    if(_launch_type == symphony::internal::with_sliding_window)  {
       auto bptr = static_cast<symphony::internal::ringbuffer<input_type>*>(_buffer);
       size_t id = bptr->get_buffer_index(i + _offset);
       return (*bptr)[id];
     } else {
       auto bptr = static_cast<symphony::internal::dynamicbuffer<input_type>*>(_buffer);
       size_t id = bptr->get_buffer_index(i + _offset);
       return (*bptr)[id];
     }
  }

  size_t get_first_elem_iter_id() const {
    return _first;
  }

  InputType& get_ith_element(size_t i) {
    SYMPHONY_API_THROW_CUSTOM(i < _size,
                          std::out_of_range,
                          "Out of range access to stage_input.");
    if(_launch_type == symphony::internal::with_sliding_window)  {
       auto bptr = static_cast<symphony::internal::ringbuffer<input_type>*>(_buffer);
       size_t id = bptr->get_buffer_index(i + _offset);
       return (*bptr)[id];
     } else {
       auto bptr = static_cast<symphony::internal::dynamicbuffer<input_type>*>(_buffer);
       size_t id = bptr->get_buffer_index(i + _offset);
       return (*bptr)[id];
     }
  }

  size_t size() const {return _size;}

  SYMPHONY_DELETE_METHOD(stage_input(stage_input const& other));
  SYMPHONY_DELETE_METHOD(stage_input(stage_input&& other));
  SYMPHONY_DELETE_METHOD(stage_input& operator=(stage_input const& other));
  SYMPHONY_DELETE_METHOD(stage_input& operator=(stage_input&& other));

  template <typename F, size_t Arity, typename RT>
  friend class symphony::internal::cpu_stage_function_arity_dispatch;
  template <typename F, size_t Arity, typename RT>
  friend class symphony::internal::gpu_stage_function_arity_dispatch;

};

class pipeline_context_base {
protected:

  size_t                              _iter_id;

  size_t                              _max_stage_iter;

  symphony::internal::pipeline_stage_instance*  _stage;

  symphony::internal::pipeline_instance_base*   _pipeline_instance;

  pipeline_context_base(size_t iter_id,
                        size_t max_stage_iter,
                        symphony::internal::pipeline_stage_instance* stage,
                        symphony::internal::pipeline_instance_base* instance):
      _iter_id(iter_id),
      _max_stage_iter(max_stage_iter),
      _stage(stage),
      _pipeline_instance(instance) {}

public:

  virtual ~pipeline_context_base() {}

  void cancel_pipeline();

  size_t get_iter_id() const {

    return _iter_id - 1;
  }

   size_t get_max_stage_iter() const  {
     return has_iter_limit() ? _max_stage_iter : 0;}

  size_t get_stage_id() const;

   bool has_iter_limit() const;

  void stop_pipeline();

  SYMPHONY_DELETE_METHOD(pipeline_context_base(pipeline_context_base const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context_base(pipeline_context_base&& other));
  SYMPHONY_DELETE_METHOD(pipeline_context_base& operator=(pipeline_context_base const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context_base& operator=(pipeline_context_base&& other));
};

template<typename... UserData> class pipeline_context;

template<>
class pipeline_context<>:public pipeline_context_base{
private:

  pipeline_context(size_t iter_id,
                   size_t max_stage_iter,
                   symphony::internal::pipeline_stage_instance* stage,
                   symphony::internal::pipeline_instance_base* inst):
    pipeline_context_base(
      iter_id, max_stage_iter, stage, inst) {}

public:

  virtual ~pipeline_context() {}

  SYMPHONY_DELETE_METHOD(pipeline_context(pipeline_context const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context(pipeline_context &&other));
  SYMPHONY_DELETE_METHOD(pipeline_context& operator=(pipeline_context const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context& operator=(pipeline_context&& other));

  template<typename F, typename ...UserData>
  friend class symphony::internal::pipeline_cpu_stage_skeleton;
  template<typename BeforeBody, typename GK, typename AfterBody, typename... UserData>
  friend class symphony::internal::pipeline_gpu_stage_skeleton;

};

template<typename UserData>
class pipeline_context<UserData>:public pipeline_context_base{
private:

  pipeline_context(size_t iter_id,
                   size_t max_stage_iter,
                   symphony::internal::pipeline_stage_instance* stage,
                   symphony::internal::pipeline_instance_base* inst):
    pipeline_context_base(
      iter_id, max_stage_iter, stage, inst) {}

public:

  virtual ~pipeline_context() {}

  UserData* get_data() const {
    return static_cast<symphony::internal::pipeline_instance<UserData>*>(
      _pipeline_instance)->_user_data;
  }

  SYMPHONY_DELETE_METHOD(pipeline_context(pipeline_context const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context(pipeline_context&& other));
  SYMPHONY_DELETE_METHOD(pipeline_context& operator=(pipeline_context const& other));
  SYMPHONY_DELETE_METHOD(pipeline_context& operator=(pipeline_context&& other));

  template<typename F, typename ...UD>
  friend class symphony::internal::pipeline_cpu_stage_skeleton;
  template<typename BeforeBody, typename GK, typename AfterBody, typename... UD>
  friend class symphony::internal::pipeline_gpu_stage_skeleton;

};

};
