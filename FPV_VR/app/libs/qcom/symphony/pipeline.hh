// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/pipelinedata.hh>

#ifdef SYMPHONY_CAPI_COMPILING_CC
#include <lib/capi.hh>
#endif
#include <symphony/internal/patterns/pipeline/pipeline.hh>
#include <symphony/internal/patterns/pipeline/pipelineutility.hh>

namespace symphony {

namespace internal {
namespace test {
  class pipeline_tester;
  class pipeline_capi_internal_tester;
};
};

class serial_stage;
class parallel_stage;
class iteration_lag;
class iteration_rate;

namespace pattern {

template<typename ...UserData>
class pipeline {

protected:

  symphony::internal::symphony_shared_ptr<symphony::internal::pipeline_skeleton<UserData...>> _skeleton;

  static constexpr size_t userdata_size = sizeof...(UserData);
  static_assert(userdata_size <= 1,
                "Symphony pipeline can only have, at most, one type of context data.");

  template<typename... Args>
  void add_cpu_stage(Args&&... args) {
    using check_params =
      typename symphony::internal::pipeline_utility::check_add_cpu_stage_params<context&, Args...>;
    static_assert(check_params::value, "add_cpu_stage has illegal parameters");

    int const stage_index = check_params::stage_index;
    int const body_index  = check_params::body_index;
    int const lag_index   = check_params::result::iteration_lag_index;
    int const rate_index  = check_params::result::iteration_rate_index;
    int const sws_index   = check_params::result::sliding_window_size_index;
    int const pattern_tuner_index   = check_params::result::pattern_tuner_index;

    bool const has_iteration_lag =
      check_params::result::iteration_lag_num == 0 ? false : true;
    using StageType       = typename check_params::StageType;

    auto input_tuple      = std::forward_as_tuple(args...);
    symphony::iteration_lag       lag(0);
    symphony::iteration_rate      rate(1, 1);
    symphony::sliding_window_size sws(0);
    symphony::pattern::tuner      t;
    auto default_tuple    = std::forward_as_tuple(lag, rate, sws, t);

    using stage_param_list_type = symphony::internal::pipeline_utility::stage_param_list_type;
    using lag_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::iteration_lag,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 lag_index,
                                                                 stage_param_list_type::symphony_iteration_lag>;
    using rate_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::iteration_rate,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 rate_index,
                                                                 stage_param_list_type::symphony_iteration_rate>;
    using sws_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::sliding_window_size,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 sws_index,
                                                                 stage_param_list_type::symphony_sliding_window_size>;

    using pattern_tuner_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::pattern::tuner,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 pattern_tuner_index,
                                                                 stage_param_list_type::symphony_stage_pattern_tuner>;

    using Body =
      typename symphony::internal::pipeline_utility::get_cpu_body_type<decltype(input_tuple), body_index>::type;

    auto skeleton = c_ptr(_skeleton);
    skeleton->template add_cpu_stage<StageType, Body>(
      std::get<stage_index>(input_tuple),
      std::forward<Body>(symphony::internal::pipeline_utility::get_tuple_element_helper<body_index,
                         decltype(input_tuple)>::get(input_tuple)),
      has_iteration_lag,
      lag_mux::get(input_tuple, default_tuple),
      rate_mux::get(input_tuple, default_tuple),
      sws_mux::get(input_tuple, default_tuple),
      pattern_tuner_mux::get(input_tuple, default_tuple));
  }

public:

  using context = pipeline_context<UserData...>;

  pipeline() :
    _skeleton(new symphony::internal::pipeline_skeleton<UserData...>()) {
  }

  virtual ~pipeline() {}

  pipeline(pipeline const& other) :
   _skeleton(
     new symphony::internal::pipeline_skeleton<UserData...>(*c_ptr(other._skeleton))) {
  }

  pipeline(pipeline && other) :_skeleton(other._skeleton) {
    other._skeleton = nullptr;
  }

  pipeline& operator=(pipeline const& other) {
    _skeleton =
      symphony::internal::symphony_shared_ptr<symphony::internal::pipeline_skeleton<UserData...>>(
        new symphony::internal::pipeline_skeleton<UserData...>(*c_ptr(other._skeleton)));

    return *this;
  }

  pipeline& operator=(pipeline&& other) {
    _skeleton = other._skeleton;
    other._skeleton = nullptr;
    return *this;
  }

  template<typename... Args>
  void add_stage(Args&&... args)  {
    add_cpu_stage(std::forward<Args>(args)...);
  }

  template<typename... Confs>
  void run(UserData*... context_data,
           size_t num_iterations,
           Confs&&... confs) const {

    using check_params =
      typename symphony::internal::pipeline_utility::check_launch_params<size_t, Confs&&...>;

    int const pattern_tuner_index   = check_params::result::pattern_tuner_index;

    auto input_tuple      = std::forward_as_tuple(num_iterations, confs...);
    symphony::pattern::tuner      t;
    auto default_tuple    = std::forward_as_tuple(num_iterations, t);

    using launch_param_list_type = symphony::internal::pipeline_utility::launch_param_list_type;
    using pattern_tuner_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::pattern::tuner,
                                                                     decltype(input_tuple),
                                                                     decltype(default_tuple),
                                                                     pattern_tuner_index,
                                                                     launch_param_list_type::symphony_launch_pattern_tuner>;

    auto skeleton = c_ptr(_skeleton);

    if (skeleton->is_empty())
      return;

    skeleton->freeze();

    symphony::internal::symphony_shared_ptr<symphony::internal::pipeline_instance<UserData...>> pinst(
      new symphony::internal::pipeline_instance<UserData...>(_skeleton));

    auto pipelineinst = c_ptr(pinst);
    pipelineinst->launch(context_data...,
                         num_iterations,
                         skeleton->get_launch_type(),
                         pattern_tuner_mux::get(input_tuple, default_tuple));

    try {
      pipelineinst->wait_for();
    } catch (...)  { }
  }

  template<typename... Confs>
  symphony::task_ptr<>
  create_task(UserData*... context_data,
              size_t num_iterations,
              Confs&&... confs) const {

    using check_params =
      typename symphony::internal::pipeline_utility::check_launch_params<size_t, Confs&&...>;

    int const pattern_tuner_index   = check_params::result::pattern_tuner_index;

    auto input_tuple      = std::forward_as_tuple(num_iterations, confs...);
    symphony::pattern::tuner      t;
    auto default_tuple    = std::forward_as_tuple(num_iterations, t);

    using launch_param_list_type = symphony::internal::pipeline_utility::launch_param_list_type;
    using pattern_tuner_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::pattern::tuner,
                                                                     decltype(input_tuple),
                                                                     decltype(default_tuple),
                                                                     pattern_tuner_index,
                                                                     launch_param_list_type::symphony_launch_pattern_tuner>;
    auto t1 = pattern_tuner_mux::get(input_tuple, default_tuple);

    auto skeleton = c_ptr(_skeleton);
    SYMPHONY_API_ASSERT(skeleton->is_empty() == false, "The pipeline has no stages.");

    skeleton->freeze();

    symphony::internal::symphony_shared_ptr<symphony::internal::pipeline_instance<UserData...>> pinst(
      new symphony::internal::pipeline_instance<UserData...>(_skeleton));

    auto datatp = std::make_tuple(context_data..., num_iterations);

    auto launch_type = skeleton->get_launch_type();

    using launch_helper =
      typename symphony::internal::pipeline_utility::apply_launch<userdata_size + 1>;

    auto ptask = symphony::create_task([pinst, datatp, launch_type, t1]() {

      auto pipelineinst = c_ptr(pinst);
      launch_helper::apply(pipelineinst, launch_type, t1, datatp);
      pipelineinst->finish_after();
    });

    return ptask;
  }

  symphony::task_ptr<void(UserData*..., size_t)>
  create_task(const symphony::pattern::tuner& t = symphony::pattern::tuner()) const {

    auto skeleton = c_ptr(_skeleton);

    SYMPHONY_API_ASSERT(skeleton->is_empty() == false, "The pipeline has no stages.");

    skeleton->freeze();
    symphony::internal::symphony_shared_ptr<symphony::internal::pipeline_instance<UserData...>> pinst(
      new symphony::internal::pipeline_instance<UserData...>(_skeleton));

    auto launch_type = skeleton->get_launch_type();

    auto ptask =
      symphony::create_task([pinst, launch_type, t](UserData*... context_data,
                                                    size_t num_iterations) {
      auto pipelineinst = c_ptr(pinst);
      pipelineinst->launch(context_data...,
                           num_iterations,
                           launch_type,
                           t);
      pipelineinst->finish_after();
    });

    return ptask;
  }

  bool is_valid() {
    auto skeleton = c_ptr(_skeleton);
    return skeleton->is_valid();
  }

  void disable_sliding_window() {
    auto skeleton = c_ptr(_skeleton);
    skeleton->set_launch_type(symphony::internal::without_sliding_window);
  }

  void enable_sliding_window() {
    auto skeleton = c_ptr(_skeleton);
    skeleton->set_launch_type(symphony::internal::with_sliding_window);
  }

  friend symphony::internal::test::pipeline_tester;
  friend symphony::internal::test::pipeline_capi_internal_tester;

#ifdef SYMPHONY_CAPI_COMPILING_CC
  friend int ::symphony_pattern_pipeline_run(
    ::symphony_pattern_pipeline_ptr p,
    void* data,
    unsigned int iterations);
#endif

};

};

symphony::task_ptr<>
create_task(const symphony::pattern::pipeline<>& p,
            size_t num_iterations,
            const symphony::pattern::tuner &t = symphony::pattern::tuner());

symphony::task_ptr<void(size_t)>
create_task(const symphony::pattern::pipeline<>& p,
            const symphony::pattern::tuner &t = symphony::pattern::tuner());

void launch(const symphony::pattern::pipeline<>& p,
            size_t num_iterations,
            const symphony::pattern::tuner &t = symphony::pattern::tuner());

template<typename UserData>
symphony::task_ptr<>
create_task(const symphony::pattern::pipeline<UserData>& p,
            UserData* context_data,
            size_t num_iterations,
            const symphony::pattern::tuner &t = symphony::pattern::tuner()) {
  return p.create_task(context_data, num_iterations, t);
}

template<typename UserData>
symphony::task_ptr<void(UserData*, size_t)>
create_task(const symphony::pattern::pipeline<UserData>& p,
            const symphony::pattern::tuner &t = symphony::pattern::tuner()) {
  return p.create_task(t);
}

template<typename UserData>
void launch(const symphony::pattern::pipeline<UserData>& p,
            UserData* context_data,
            size_t num_iterations,
            const symphony::pattern::tuner &t = symphony::pattern::tuner()) {
  p.run(context_data, num_iterations, t);
}
};

#ifdef SYMPHONY_HAVE_GPU
namespace symphony {
namespace beta {
namespace pattern {

template<typename ...UserData>
class pipeline : public symphony::pattern::pipeline<UserData...> {

  static constexpr size_t userdata_size = sizeof...(UserData);
  static_assert(userdata_size <= 1,
                "Symphony pipeline can only have, at most, one type of context data.");
  using parent_type = typename symphony::pattern::pipeline<UserData...>;

  template<typename... Args>
  void add_gpu_stage(Args&&... args) {
    using check_gpu_kernel = typename symphony::internal::pipeline_utility::check_gpu_kernel<Args...>;
    static_assert(check_gpu_kernel::has_gpu_kernel,
      "gpu stage should have a gpu kernel");

    auto input_tuple      = std::forward_as_tuple(std::forward<Args>(args)...);

    using check_params =
      typename symphony::internal::pipeline_utility::check_add_gpu_stage_params<
        context&, decltype(input_tuple), Args...>;
    static_assert(check_params::value, "add_gpu_stage has illegal parameters");

    int const stage_index = check_params::stage_index;
    int const lag_index   = check_params::result::iteration_lag_index;
    int const rate_index  = check_params::result::iteration_rate_index;
    int const sws_index   = check_params::result::sliding_window_size_index;
    int const pattern_tuner_index   = check_params::result::pattern_tuner_index;
    bool const has_iteration_lag =
      check_params::result::iteration_lag_num == 0 ? false : true;
    using StageType       = typename check_params::StageType;

    symphony::iteration_lag       lag(0);
    symphony::iteration_rate      rate(1, 1);
    symphony::sliding_window_size sws(0);
    symphony::pattern::tuner      t;
    auto default_tuple    = std::forward_as_tuple(lag, rate, sws, t);

    using stage_param_list_type = symphony::internal::pipeline_utility::stage_param_list_type;
    using lag_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::iteration_lag,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 lag_index,
                                                                 stage_param_list_type::symphony_iteration_lag>;
    using rate_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::iteration_rate,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 rate_index,
                                                                 stage_param_list_type::symphony_iteration_rate>;
    using sws_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::sliding_window_size,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 sws_index,
                                                                 stage_param_list_type::symphony_sliding_window_size>;

    using pattern_tuner_mux =
      typename symphony::internal::pipeline_utility::mux_param_value<symphony::pattern::tuner,
                                                                 decltype(input_tuple),
                                                                 decltype(default_tuple),
                                                                 pattern_tuner_index,
                                                                 stage_param_list_type::symphony_stage_pattern_tuner>;

    const auto bbody_index  = check_params::bbody_index;
    using BeforeBody        = typename check_params::BeforeBody;

    const auto abody_index  = check_params::abody_index;
    using AfterBody         = typename check_params::AfterBody;

    const auto gkbody_index = check_params::gkbody_index;
    using GKBody            = typename check_params::GKBody;

    static_assert(gkbody_index != -1, "gpu stage should have a gpu kernel");

    c_ptr(parent_type::_skeleton)->template add_gpu_stage<StageType, BeforeBody, GKBody, AfterBody>(
      std::get<stage_index>(input_tuple),
      std::forward<BeforeBody>(symphony::internal::pipeline_utility::get_tuple_element_helper<bbody_index,
                           decltype(input_tuple)>::get(input_tuple)),
      std::forward<GKBody>(symphony::internal::pipeline_utility::get_tuple_element_helper<gkbody_index,
                           decltype(input_tuple)>::get(input_tuple)),
      std::forward<AfterBody>(symphony::internal::pipeline_utility::get_tuple_element_helper<abody_index,
                           decltype(input_tuple)>::get(input_tuple)),
      has_iteration_lag,
      lag_mux::get(input_tuple, default_tuple),
      rate_mux::get(input_tuple, default_tuple),
      sws_mux::get(input_tuple, default_tuple),
      pattern_tuner_mux::get(input_tuple, default_tuple));
  }

public:

  using context = typename parent_type::context;

  pipeline() : parent_type() {}

  virtual ~pipeline() {}

  pipeline(pipeline const& other) : parent_type(other) {}

  pipeline(pipeline && other) :
    parent_type(std::forward<parent_type>(other)) {}

  pipeline& operator=(pipeline const& other) {
    parent_type::operator=(other);
    return *this;
  }

  pipeline& operator=(pipeline&& other) {
    parent_type::operator=(std::forward<parent_type>(other));
    return *this;
  }

  template<typename... Args>
  typename std::enable_if<
             !internal::pipeline_utility::check_gpu_kernel<
               Args...
             >::has_gpu_kernel, void
            >::type
  add_stage(Args&&... args)  {
    parent_type::add_cpu_stage(std::forward<Args>(args)...);
  }

  template<typename... Args>
  typename std::enable_if<
             internal::pipeline_utility::check_gpu_kernel<
               Args...
             >::has_gpu_kernel, void
            >::type
  add_stage(Args&&... args)  {
    add_gpu_stage(std::forward<Args>(args)...);
  }

  friend symphony::internal::test::pipeline_tester;

};

};
};
};
#endif
