// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <vector>

#include <symphony/internal/util/macros.hh>
#include <symphony/internal/patterns/pipeline/pipelineskeleton.hh>

namespace symphony {
namespace internal {

template<symphony::mem_order const mem_order>
struct comp_result_w_total_iters;

template<>
struct comp_result_w_total_iters<symphony::mem_order_relaxed>  {
  static void comp(size_t& result, size_t total_iters)  {
    result = result > total_iters ? total_iters : result;
  }
};

template<>
struct comp_result_w_total_iters<symphony::mem_order_seq_cst>  {
  static void comp(size_t& result, size_t total_iters)  {
    if(total_iters > 0)
      result = result > total_iters ? total_iters : result;
  }
};

class pipeline_stage_instance{

protected:

  size_t                         _data_buf_size;

  size_t const                   _doc;

  size_t const                   _id;

  size_t                         _iter_ahead;

  size_t const                   _lag;

  size_t                         _lag_succ;

  size_t                         _lag_succ_iter_weight;

  size_t                         _lag_succ_iter_offset;

  size_t                         _last_token_consumed;

  size_t                         _last_token_released;

  std::atomic<size_t>            _last_token_passed;

  pipeline_instance_base*        _pipeline_instance;

  size_t                         _rate_curr;

  size_t                         _rate_succ;

  pipeline_stage_skeleton_base*  _stage_skeleton_ptr;

  std::atomic<size_t>            _total_iters;

  pipeline_stage_type const      _type;

  size_t                         _iters_active;
  size_t                         _iters_done;
  std::mutex                     _mutex;
  bool*                          _done;
  size_t                         _max_doc_rate;
  size_t                         _min_init_iters;
  pipeline_stage_instance*       _next;
  pipeline_stage_instance*       _prev;
  bool                           _split_stage_work;
  size_t                         _chunk_size;

public:

  pipeline_stage_instance(pipeline_stage_skeleton_base* stage_ptr,
                          pipeline_instance_base* pinst):
    _data_buf_size(0),
    _doc(stage_ptr->_doc),
    _id(stage_ptr->_id),
    _iter_ahead(0),
    _lag(stage_ptr->_lag),
    _lag_succ(0),
    _lag_succ_iter_weight(0),
    _lag_succ_iter_offset(0),
    _last_token_consumed(1),
    _last_token_released(0),
    _last_token_passed(0),
    _pipeline_instance(pinst),
    _rate_curr(0),
    _rate_succ(0),
    _stage_skeleton_ptr(stage_ptr),
    _total_iters(0),
    _type(stage_ptr->_type),
    _iters_active(0),
    _iters_done(0),
    _mutex(),
    _done(nullptr),
    _max_doc_rate(0),
    _min_init_iters(0),
    _next(nullptr),
    _prev(nullptr),
    _split_stage_work(true),
    _chunk_size(0)
  {
    if(is_serial())
      _split_stage_work = false;

    auto tuner_chunk_size = _stage_skeleton_ptr->_tuner.get_chunk_size();

    if((is_parallel() && tuner_chunk_size > 1) || is_serial())
      _chunk_size = tuner_chunk_size;

    if(is_serial() && stage_ptr->_hetero_type != pipeline_stage_hetero_type::gpu)
      _iter_ahead = tuner_chunk_size;
    else
      _iter_ahead = stage_ptr->_doc;

    _done = new bool[_iter_ahead];
    memset(_done, 0, sizeof(bool) * _iter_ahead);

    SYMPHONY_INTERNAL_ASSERT(_iter_ahead != 0, "Stage degree of concurrency cannot be 0.");
  }

  virtual ~pipeline_stage_instance() {
    delete[] _done;
  }

  void apply(size_t in_first_idx,
             size_t in_size,
             stagebuffer* in_buf,
             size_t out_idx,
             stagebuffer* out_buf,
             size_t iter_id,
             pipeline_launch_type launch_type) {
    auto total_iters = _total_iters.load(symphony::mem_order_relaxed);

    _stage_skeleton_ptr->apply(
      in_first_idx,
      in_size,
      in_buf,
      out_idx,
      out_buf,
      iter_id,
      total_iters,
      this,
      _pipeline_instance,
      launch_type);
  };

#ifdef SYMPHONY_HAVE_GPU

  std::pair<task_ptr<>, void*> apply_before(
    size_t in_first_idx,
    size_t in_size,
    stagebuffer* in_buf,
    size_t iter_id,
    pipeline_launch_type launch_type) {

      auto total_iters = _total_iters.load(symphony::mem_order_relaxed);

      return _stage_skeleton_ptr->apply_before(
        in_first_idx,
        in_size,
        in_buf,
        iter_id,
        total_iters,
        this,
        _pipeline_instance,
        launch_type);
  };

  void apply_after(
    size_t out_idx,
    stagebuffer* out_buf,
    size_t iter_id,
    pipeline_launch_type launch_type,
    void* gk_tp) {
      auto total_iters = _total_iters.load(symphony::mem_order_relaxed);
      _stage_skeleton_ptr->apply_after(
        out_idx,
        out_buf,
        iter_id,
        total_iters,
        this,
        _pipeline_instance,
        launch_type,
        gk_tp);
  };
#endif

  void alloc_stage_buf(stagebuffer*& buf,
                       size_t size,
                       pipeline_launch_type type) {
    _stage_skeleton_ptr->alloc_stage_buf(buf, size, type);
  };

  void free_stage_buf(stagebuffer*& buf) {
    _stage_skeleton_ptr->free_stage_buf(buf);
  }

  size_t get_return_type() const {
    return _stage_skeleton_ptr->get_return_type();
  }

  size_t get_arg_type() const {
    return _stage_skeleton_ptr->get_arg_type();
  }

  template<symphony::mem_order const  mem_order>
  size_t fetch_some_work(pipeline_launch_type launch_type) const {

    if(_doc == 1 && _iters_active > _iters_done) {
      return 0;
    }

    size_t from_prev = 0;
    size_t result = std::numeric_limits<size_t>::max();
    auto prev_stage = _prev;

    if(prev_stage) {
      from_prev = prev_stage->template get_num_iters_for_next_stage_can_be_done<mem_order>();
      result = from_prev;
    }

    size_t from_next = 0;
    auto next_stage = _next;

    if(next_stage && launch_type == with_sliding_window) {
      from_next =
        ((next_stage->_iters_done + _lag_succ + 1) *
         _rate_curr - 1) / _rate_succ + _iter_ahead;

      from_next = from_next > _min_init_iters ? from_next : _min_init_iters;

      result = result > from_next ? from_next : result;
    }

    auto total_iters = _total_iters.load(mem_order);
    comp_result_w_total_iters<mem_order>::comp(result, total_iters);

    result = result > _iters_done + _iter_ahead ? _iters_done + _iter_ahead : result;
    result = result > _iters_active ? result - _iters_active : 0;

    if (_chunk_size == 0)
      return (result + 1) >> 1;

    return result > _chunk_size ? _chunk_size : result;
  }

  template<symphony::mem_order const mem_order>
  size_t get_num_iters_for_next_stage_can_be_done() const {

    size_t const iters_done = _iters_done;
    size_t scaled_iters_done = iters_done * _rate_succ / _rate_curr;
    size_t total_iters = _total_iters.load(mem_order);

    if(total_iters > 0 && iters_done >= total_iters) {
      return scaled_iters_done;
    }

    if(scaled_iters_done < _lag_succ)
      return 0;

    return scaled_iters_done - _lag_succ;
  }

  template<symphony::mem_order const mem_order>
  bool is_finished() const {
    size_t total_iters = _total_iters.load(mem_order);

    return (total_iters > 0) && (_iters_done >= total_iters);
  }

  bool is_parallel() const {
    return (_type == pipeline_stage_type::parallel) && (_doc > 1);
  }

  bool is_serial() const {
    return _type == pipeline_stage_type::serial_in_order ||
     _type == pipeline_stage_type::serial_out_of_order || _doc == 1;
  }

  bool is_serial_in_order() const {
    return _type == pipeline_stage_type::serial_in_order;
  }

  bool is_serial_out_of_order() const {
    return _type == pipeline_stage_type::serial_out_of_order;
  }

  SYMPHONY_DELETE_METHOD(pipeline_stage_instance(pipeline_stage_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_stage_instance(pipeline_stage_instance&& other));
  SYMPHONY_DELETE_METHOD(pipeline_stage_instance& operator=(pipeline_stage_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_stage_instance& operator=(pipeline_stage_instance&& other));

  friend class pipeline_instance_base;
  friend class ::symphony::pipeline_context_base;
};

class pipeline_instance_base{
  static size_t const                      _max_active_tasks;
  static size_t const                      _aggressive_fetch_cutoff_num_iters;

protected:
  symphony::group_ptr                     _group;
  std::atomic<bool>                       _is_stopped;
  bool                                    _launch_with_iterations;
  size_t                                  _num_stages;
  size_t                                  _num_tokens;
  pipeline_skeleton_base*                 _skeleton;
  std::vector<pipeline_stage_instance*>   _stages;
  std::vector<stagebuffer*>               _stage_buf_ptrs;
  bool                                    _split_serial_stage_work;

  std::atomic<size_t>                     _active_tasks;
  std::atomic<bool>                       _pipeline_finished;
#ifdef SYMPHONY_USE_PIPELINE_LOGGER
  std::atomic<size_t>                     _max_tasks;
  std::atomic<size_t>                     _num_tasks;
#endif

public:

  explicit pipeline_instance_base(pipeline_skeleton_base* p):
    _group(nullptr),
    _is_stopped(false),
    _launch_with_iterations(false),
    _num_stages(p->_num_stages),
    _num_tokens(0),
    _skeleton(p),
    _stages(),
    _stage_buf_ptrs(p->_num_stages),
    _split_serial_stage_work(false),
    _active_tasks(0),
    _pipeline_finished(false)
#ifdef SYMPHONY_USE_PIPELINE_LOGGER
    , _max_tasks(0),
    _num_tasks(0),
    _task_counts()
#endif
  {
    for(size_t i = 0; i < _skeleton->_stages.size(); i ++) {
      _stages.push_back(new pipeline_stage_instance(_skeleton->_stages[i], this));
      if(i == 0)
       continue;
      _stages[i]->_prev = _stages[i-1];
      _stages[i-1]->_next = _stages[i];
    }

    _group = symphony::create_group("pipieline instance.");
  };

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

  ~pipeline_instance_base() {
    for(auto stage: _stages) {
      delete _stage_buf_ptrs[stage->_id];
      delete stage;
    }
    _stages.clear();
  }
SYMPHONY_GCC_IGNORE_END("-Weffc++");

  void check_stage_io_type() const;

  void check_stage_sliding_window_size() const;

  void cancel() {
    _group->cancel();
  }

  bool is_first_stage_serial() {
    SYMPHONY_INTERNAL_ASSERT(_stages.size() > 0, "Pipeline cannot be empty.");
    return _stages[0]->is_serial();
  }

  void perform_cpu_stage(
    size_t stage_id,
    size_t iter_id,
    pipeline_launch_type launch_type,
    std::memory_order mem_order);

#ifdef SYMPHONY_HAVE_GPU

  template<const std::memory_order mem_order>
  void perform_gpu_stage(
    size_t stage_id,
    size_t iter_id,
    pipeline_launch_type launch_type) {

    auto tp = prepare_gpu_stage(stage_id, iter_id, launch_type, mem_order);

    auto gpu_tptr           = std::get<0>(tp);
    auto after_body_tp_ptr = std::get<1>(tp);

    _group->launch(gpu_tptr);

    auto cpu_tptr = symphony::create_task(
      [stage_id, iter_id, launch_type, after_body_tp_ptr, this] {
      postprocess_gpu_stage(stage_id, iter_id, launch_type, mem_order, after_body_tp_ptr);
      if(get_load_on_current_thread() == 0) {
         _active_tasks ++;
  #ifdef SYMPHONY_USE_PIPELINE_LOGGER
         _num_tasks.fetch_add(1, symphony::mem_order_relaxed);
  #endif

        size_t first_search_stage =
          stage_id == _num_stages - 1 ? _num_stages : stage_id + 2;
        pipeline_task_aggressive_fetch<mem_order>(first_search_stage, launch_type);
      }
    });

    gpu_tptr>>cpu_tptr;
    _group->launch(cpu_tptr);
  }

  std::pair<task_ptr<>, void*> prepare_gpu_stage(
    size_t stage_id,
    size_t iter_id,
    pipeline_launch_type launch_type,
    std::memory_order mem_order);

  void postprocess_gpu_stage(
    size_t stage_id,
    size_t iter_id,
    pipeline_launch_type launch_type,
    std::memory_order mem_order,
    void* gk_tp);
#endif

  size_t gcd_stage_iter_rate(size_t i) {
    size_t a = _stages[i]->_stage_skeleton_ptr->_rate_curr;
    size_t b = _stages[i]->_stage_skeleton_ptr->_rate_pred;

    SYMPHONY_INTERNAL_ASSERT(a != 0, "Stage iter rate cannot be zero.");
    SYMPHONY_INTERNAL_ASSERT(b != 0, "Stage iter rate cannot be zero.");

    return pipeline_utility::get_gcd(a, b);
  }

  void get_predecessor_iter_info(size_t stage_id,
                                 size_t curr_iter,
                                 size_t& pred_input_biter,
                                 size_t& pred_input_eiter,
                                 size_t& pred_input_size,
                                 symphony::mem_order mem_order) const {

    if(stage_id == 0) {
      pred_input_biter = 0;
      pred_input_eiter = 0;
      pred_input_size = 0;
      return;
    }

    auto pred_stage = _stages[stage_id - 1];
    auto curr_stage = _stages[stage_id];
    size_t curr_lag = curr_stage->_lag;
    size_t a = pred_stage->_rate_curr;
    size_t b = pred_stage->_rate_succ;

    auto pred_total_iters = pred_stage->_total_iters.load(mem_order);

    if(a <= b) {
      size_t pred_launch_iter =
        pipeline_utility::get_pred_iter(a, b, curr_iter + curr_lag);
      size_t pred_launch_iter_wo_lag =
        pipeline_utility::get_pred_iter(a, b, curr_iter);

      pred_input_biter = pred_launch_iter_wo_lag;

      pred_input_eiter = pred_total_iters == 0 ?
        pred_launch_iter :
        pred_launch_iter <= pred_total_iters ? pred_launch_iter : pred_total_iters;

    } else {
      size_t pred_launch_iter =
        pipeline_utility::get_pred_iter(a, b, curr_iter + curr_lag);
      size_t pred_launch_prev_iter_wo_lag =
        pipeline_utility::get_pred_iter(a, b, curr_iter - 1);

      pred_input_biter = pred_launch_prev_iter_wo_lag + 1;

      pred_input_eiter = pred_total_iters == 0 ?
        pred_launch_iter :
        pred_launch_iter <= pred_total_iters ? pred_launch_iter : pred_total_iters;
    }
    pred_input_size = pred_input_eiter - pred_input_biter + 1;
  }

  void initialize_stages();

  void initialize_stb(
    pipeline_launch_type launch_type);

  template<symphony::mem_order const mem_order>
  void pipeline_task_aggressive_fetch(size_t               init_stage,
                                      pipeline_launch_type launch_type) {

    size_t count = 0;
    size_t first_search_stage = init_stage;
    size_t const aggressive_fetch_cutoff_num_iters = 4;

    while(1) {
      bool found_work = false;

      for(size_t i = first_search_stage; i > 0; i --) {

        auto curr_stage = _stages[i-1];

        if(curr_stage->_mutex.try_lock() == false)
          continue;

        size_t fetch_work = curr_stage->template fetch_some_work<mem_order>(launch_type);

        if(fetch_work ==  0) {
          curr_stage->_mutex.unlock();
          continue;
        }

        found_work = true;
        size_t first_iter = curr_stage->_iters_active + 1;
        curr_stage->_iters_active += fetch_work;
        curr_stage->_mutex.unlock();

#ifdef SYMPHONY_HAVE_GPU
        if(curr_stage->_stage_skeleton_ptr->_hetero_type == pipeline_stage_hetero_type::gpu) {
           const size_t last_iter = first_iter + fetch_work;

           size_t total_iters = _stages[i-1]->_total_iters.load(mem_order);
           for(size_t y = first_iter; (total_iters == 0 || y <= total_iters) && y < last_iter; ++ y) {
             perform_gpu_stage<mem_order>(i - 1, y, launch_type);
             total_iters = _stages[i-1]->_total_iters.load(mem_order);
          }
          continue;
        }
#endif

        size_t const last_iter = first_iter + fetch_work;
        size_t y_done = first_iter;

        size_t total_iters = _stages[i-1]->_total_iters.load(mem_order);
        for(; (total_iters == 0 || y_done <= total_iters) && y_done < last_iter; ++ y_done) {
          perform_cpu_stage(i - 1, y_done, launch_type, mem_order);
          total_iters = _stages[i-1]->_total_iters.load(mem_order);
        }

        SYMPHONY_INTERNAL_ASSERT(!curr_stage->is_parallel() || fetch_work <= curr_stage->_doc,
          "fetch too much work.");

        if(curr_stage->is_parallel()) {
          std::lock_guard<std::mutex> l(curr_stage->_mutex);

          for(size_t y = first_iter; y < y_done; ++ y)
            curr_stage->_done[y % curr_stage->_doc] = true;

          size_t inc_iters_done = curr_stage->_iters_done + 1;

          while(curr_stage->_done[(inc_iters_done) % curr_stage->_doc] == true) {
            curr_stage->_done[inc_iters_done % curr_stage->_doc] = false;
            inc_iters_done ++;
          }
          curr_stage->_iters_done = inc_iters_done - 1;
        } else {
          std::lock_guard<std::mutex> l(curr_stage->_mutex);
          curr_stage->_iters_done += y_done - first_iter;
        }

        first_search_stage = i == _num_stages ? _num_stages : i + 1;

        if(get_load_on_current_thread() == 0) {
          _active_tasks ++;
#ifdef SYMPHONY_USE_PIPELINE_LOGGER
          _num_tasks.fetch_add(1, symphony::mem_order_relaxed);
#endif
          _group->launch([this, i, launch_type]{
              pipeline_task_aggressive_fetch<mem_order>(i, launch_type);
            });
        }
        break;
      }

      if(found_work) {
        count = 0;
        continue;
      }

      count ++;

      if(count < aggressive_fetch_cutoff_num_iters)
        continue;

      auto new_active_tasks = -- _active_tasks;

      if(new_active_tasks == 0  && !_stages[_num_stages-1]->template is_finished<mem_order>()) {
        first_search_stage = _num_stages;
        count = 0;

        _active_tasks.store(1, symphony::mem_order_relaxed);

        continue;
      }

      break;
    }
  }

  template<symphony::mem_order const mem_order>
  void launch_stb(size_t num_iterations,
                  pipeline_launch_type launch_type,
                  symphony::pattern::tuner const& t) {

    check_stage_io_type();

    _skeleton->preprocess();

    initialize_stb(launch_type);

    if(mem_order == symphony::mem_order_relaxed)
      setup_stage_total_iterations(num_iterations);

#ifdef SYMPHONY_USE_PIPELINE_LOGGER
    _task_counts[_active_tasks.load()] ++;
#endif

    _active_tasks = 1;

#ifdef SYMPHONY_USE_PIPELINE_LOGGER
    _num_tasks.fetch_add(1, symphony::mem_order_relaxed);
#endif

    size_t num_init_tasks = is_first_stage_serial() ? 1 : t.get_doc();

    for(size_t i = 0; i < num_init_tasks; i ++)
      _group->launch([this, launch_type]{
        pipeline_task_aggressive_fetch<mem_order>(1, launch_type);});

#ifdef SYMPHONY_USE_PIPELINE_LOGGER
    SYMPHONY_ALOG("\033[31m *** creating %zu task, max %zu at one time *** \033[0m\n",
      _num_tasks.load(), _max_tasks.load());
#endif
  }

  void setup_stage_total_iterations(size_t num_iterations);

  void set_split_serial_stage_work(bool s) {
    _split_serial_stage_work = s;
  }

  SYMPHONY_DELETE_METHOD(pipeline_instance_base(pipeline_instance_base const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance_base(pipeline_instance_base&& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance_base& operator=(pipeline_instance_base const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance_base& operator=(pipeline_instance_base&& other));

  friend class ::symphony::pipeline_context_base;
};

template<typename... UserData> class pipeline_instance;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<>
class pipeline_instance<>:
  public pipeline_instance_base,
  public ref_counted_object<pipeline_instance<>,
                            symphonyptrs::default_logger>{
SYMPHONY_GCC_IGNORE_END("-Weffc++");

  symphony_shared_ptr<pipeline_skeleton<>> _skeleton_shared_ptr;

public:

  explicit pipeline_instance(symphony_shared_ptr<pipeline_skeleton<>> p):
    pipeline_instance_base(static_cast<pipeline_skeleton_base*>(c_ptr(p))),
    _skeleton_shared_ptr(p)
  {};

  virtual ~pipeline_instance() {
  }

  void finish_after() {
    symphony::finish_after(_group);
  }

  void launch(size_t num_iterations,
              pipeline_launch_type launch_type,
              const symphony::pattern::tuner& t) {

    if(launch_type == without_sliding_window) {
      SYMPHONY_API_ASSERT(!_skeleton->_has_sliding_window,
        "Cannot launch without sliding window because some stages are using sliding windows.");
    }

    if(num_iterations == 0) {
      _launch_with_iterations = false;
      launch_stb<symphony::mem_order_seq_cst>(0, launch_type, t);
    }
    else {
      _launch_with_iterations = true;
      launch_stb<symphony::mem_order_relaxed>(num_iterations, launch_type, t);
    }
  }

  void wait_for() {
    _group->wait_for();
  }

  SYMPHONY_DELETE_METHOD(pipeline_instance(pipeline_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance(pipeline_instance&& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance& operator=(pipeline_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance& operator=(pipeline_instance&& other));

  friend class pipeline_context<>;
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");

template<typename UserData>
class pipeline_instance<UserData> :
  public pipeline_instance_base,
  public ref_counted_object<pipeline_instance<UserData>,
                            symphonyptrs::default_logger>{
SYMPHONY_GCC_IGNORE_END("-Weffc++");

private:

  symphony_shared_ptr<pipeline_skeleton<UserData>> _skeleton_shared_ptr;
  UserData*                                    _user_data;

public:

  explicit pipeline_instance(symphony_shared_ptr<pipeline_skeleton<UserData>> p):
    pipeline_instance_base(static_cast<pipeline_skeleton_base*>(c_ptr(p))),
    _skeleton_shared_ptr(p),
    _user_data(nullptr) {}

  virtual ~pipeline_instance() {}

  void finish_after() {
    symphony::finish_after(_group);
  }

  void launch(UserData* context_data,
              size_t num_iterations,
              pipeline_launch_type launch_type,
              const symphony::pattern::tuner& t) {

    if(launch_type == without_sliding_window) {
      SYMPHONY_API_ASSERT(!_skeleton->_has_sliding_window,
        "Cannot launch without sliding window because some stages are using sliding windows.");
    }

    _user_data = context_data;

    if(num_iterations == 0) {
      _launch_with_iterations = false;
      launch_stb<symphony::mem_order_seq_cst>(0, launch_type, t);
    }
    else {
      _launch_with_iterations = true;
      launch_stb<symphony::mem_order_relaxed>(num_iterations, launch_type, t);
    }
  }

  void wait_for() {
    _group->wait_for();
  }

  SYMPHONY_DELETE_METHOD(pipeline_instance(pipeline_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance(pipeline_instance&& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance& operator=(pipeline_instance const& other));
  SYMPHONY_DELETE_METHOD(pipeline_instance& operator=(pipeline_instance&& other));

  friend class pipeline_context<UserData>;
};

};
};
