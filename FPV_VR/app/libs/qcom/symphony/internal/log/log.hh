// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#if defined(__ANDROID__) || defined(__linux__)
#include <symphony/internal/log/ftracelogger.hh>
#endif

#include <symphony/internal/log/corelogger.hh>
#include <symphony/internal/log/eventcounterlogger.hh>
#include <symphony/internal/log/infrastructure.hh>
#include <symphony/internal/log/pforlogger.hh>
#include <symphony/internal/log/schedulerlogger.hh>
#include <symphony/internal/log/userhandlers.hh>
#include <symphony/internal/log/userlogapi.hh>

namespace symphony {
namespace internal {
namespace log {

#if defined(__ANDROID__) || defined(__linux__)
typedef infrastructure<event_counter_logger, ftrace_logger, pfor_logger, schedulerlogger> loggers;
#else
typedef infrastructure<event_counter_logger, pfor_logger, schedulerlogger> loggers;
#endif

typedef loggers::enabled enabled;
typedef loggers::group_id group_id;
typedef loggers::task_id task_id;

void init();
void shutdown();

void pause();
void resume();
void dump();

#ifdef SYMPHONY_LOG_FIRE_EVENT
template<typename Event, typename... Args>
inline void fire_event (Args...args) {
  auto e = Event(std::forward<Args>(args)...);
  loggers::event_fired(e);
  user_handlers::event_fired(std::forward<Event>(e));
}
#else
template<typename Event, typename... Args>
inline void fire_event (Args...) {
}
#endif

};
};
};
