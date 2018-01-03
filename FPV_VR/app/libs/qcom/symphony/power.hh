// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <chrono>
#include <string>
#include <symphony/devicetypes.hh>
#include <symphony/power/power.h>
#include <symphony/power/types.h>

#include <symphony/internal/util/strprintf.hh>

namespace symphony {
namespace power {

using frequency_type = ::symphony_power_freq_t;
using freq_percent_type = symphony_power_freq_percent_t;
using goal_type = symphony_power_goal_t;
using tolerance_type = symphony_power_tolerance_t;

namespace mode {

class efficient_t {};
class normal_t {};
class perf_burst_t {};
class saver_t {};

const efficient_t efficient {};

const normal_t normal {};

const perf_burst_t perf_burst {};

const saver_t saver {};

class window {

public :

  explicit window(freq_percent_type min = 0, freq_percent_type max = 100);

  explicit window(window const& other);

  explicit window(window&& other);

  window& operator= (window const& other);

  SYMPHONY_DELETE_METHOD(window& operator= (window&& other));

  freq_percent_type get_min() const { return _min; }

  freq_percent_type get_max() const { return _max; }

  bool is_set() const {return (_min != 0 || _max != 100); }
private :

    freq_percent_type _min;
    freq_percent_type _max;
};

};

bool is_supported();

bool request_mode(mode::window const& win,
                  std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                  device_set const& devices = {symphony::device_type::cpu});

bool request_mode(mode::window const& win, device_set const& devices);

bool request_mode(mode::efficient_t const&,
                  std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                  device_set const& devices = {symphony::device_type::cpu});

bool request_mode(mode::efficient_t const&, device_set const& devices);

bool request_mode(mode::normal_t const&, device_set const& devices = {symphony::device_type::cpu});

bool request_mode(mode::saver_t const&,
                  std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                  device_set const& devices = {symphony::device_type::cpu});

bool request_mode(mode::saver_t const&, device_set const& devices);

bool request_mode(mode::perf_burst_t const&,
                  std::chrono::milliseconds const& duration = std::chrono::milliseconds(0),
                  device_set const& devices = {symphony::device_type::cpu});

bool request_mode(mode::perf_burst_t const&, device_set const& devices);

void set_goal(goal_type desired,
              tolerance_type tolerance = 0.0,
              device_set const& devices = {symphony::device_type::cpu_big});

void clear_goal();

void regulate(goal_type measured);

class goal_performance {
public:
  goal_performance() :
    _regulation_steps(0),
    _goal_fraction(0.0),
    _average_error(0.0),
    _mean_squared_error(0.0),
    _normalized_mserror(0.0)
  {}

  goal_performance(size_t arg_regulation_steps,
                   double arg_goal_fraction,
                   double arg_average_error,
                   double arg_mean_squared_error,
                   double arg_normalized_mserror) :
    _regulation_steps(arg_regulation_steps),
    _goal_fraction(arg_goal_fraction),
    _average_error(arg_average_error),
    _mean_squared_error(arg_mean_squared_error),
    _normalized_mserror(arg_normalized_mserror)
  {}

  inline size_t regulation_steps() const { return _regulation_steps; }
  inline double goal_fraction() const { return _goal_fraction; }
  inline double average_error() const { return _average_error; }
  inline double mean_squared_error() const { return _mean_squared_error; }
  inline double normalized_mserror() const { return _normalized_mserror; }

  std::string to_string() const {
    auto s = symphony::internal::strprintf("steps=%zu goal fr=%f err=%f mse=%f nmse=%f",
                                           _regulation_steps,
                                           _goal_fraction,
                                           _average_error,
                                           _mean_squared_error,
                                           _normalized_mserror);
    return s;
  }
private:
  size_t _regulation_steps;
  double _goal_fraction;
  double _average_error;
  double _mean_squared_error;
  double _normalized_mserror;
};

goal_performance get_goal_performance();

};

namespace beta {
namespace power {

class tuner {
public:
  symphony::device_set devices;
};

void set_goal(const float, const float, const tuner&);
void clear_goal();
void regulate(const float);

};
};

};
