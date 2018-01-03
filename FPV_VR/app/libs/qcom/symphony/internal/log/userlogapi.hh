// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/log/events.hh>
#include <symphony/internal/log/loggerbase.hh>
#include <symphony/internal/log/userhandlers.hh>

namespace symphony {
namespace internal {
namespace log {

  template<typename Event>
  handler_id register_event_handler (typename Event::handler_func && handler) {
    typedef typename Event::handler_func handler_func;
    return user_handlers::add<Event>(std::forward<handler_func>(handler));
  }

  template<typename Event>
  inline bool unregister_event_handler (handler_id id) {
    return user_handlers::remove_one_for_event<Event>(id);
  }

  template<typename Event>
  inline void unregister_all_event_handlers () {
    user_handlers::remove_all_for_event<Event>();
  }

  inline void unregister_all_event_handlers () {
    user_handlers::remove_all();
  }

  template <typename UserType>
  inline void add_log_entry (UserType &p,
                             std::function < std::string (UserType*)> func)
  {
    event_context context;
    auto count = context.get_count();
    auto pos =  count % fast_buffer::s_size;
    auto& entry = fast_buffer::get_default_buffer()[pos];

    entry.reset(count, events::user_log_event<UserType>::get_id(),
                context.get_this_thread_id(),
                logger_base::logger_id::userlogger);

    static_assert(sizeof(events::user_log_event<UserType>) <= buffer_entry::s_payload_size,
                  "Userlog is larger than entry payload.");

    events::user_log_event <UserType>* loc =
      reinterpret_cast <events::user_log_event <UserType>*> (entry.get_buffer());
    new (entry.get_buffer()) events::user_log_event<UserType>(std::forward<UserType>(p));
    loc->func = func;
}

  inline void add_log_entry (std::string &s)
  {
    event_context context;

    auto count = context.get_count();
    auto pos =  count % fast_buffer::s_size;
    auto& entry = fast_buffer::get_default_buffer()[pos];

    entry.reset(count, events::user_string_event::get_id(),
                context.get_this_thread_id(),
                logger_base::logger_id::userlogger);

    static_assert(sizeof(s) <= buffer_entry::s_payload_size,
                "Userlog is larger than entry payload.");

    std::strcpy (reinterpret_cast<char*>(entry.get_buffer()), s.c_str());
  }

};
};
};
