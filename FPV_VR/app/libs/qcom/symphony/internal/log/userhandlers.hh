// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <list>

#include <symphony/internal/log/common.hh>
#include <symphony/internal/log/eventids.hh>
#include <symphony/internal/log/eventcontext.hh>

namespace symphony {
namespace internal {
namespace log {

struct user_handler_list_base {
  virtual void unregister_all() = 0;
  virtual ~user_handler_list_base(){};
};

template<typename Event>
class user_handler_list;

class user_handlers {
  typedef std::array<user_handler_list_base*,
                     events::event_id_list::num_events> handlers;

  static handlers* s_handlers;

  template<typename Event>
  static auto get_user_handler_list() -> user_handler_list<Event>* {
    return static_cast<user_handler_list<Event>*>((*s_handlers)[Event::get_id()]);
  }

  template<typename Event>
  static auto create_user_handler_list() -> user_handler_list<Event>* {
    auto list = new user_handler_list<Event>();
    (*s_handlers)[Event::get_id()] = list;
    return list;
  }

public:

  static void init();

  template<typename Event>
  static void event_fired(Event&& event) {
    auto list  = get_user_handler_list<Event>();
    if (!list)
      return;

    list->event_fired(std::forward<Event>(event));
  }

  template<typename Event>
  static handler_id add(typename Event::handler_func&& handler) {
     auto list = get_user_handler_list<Event>();

     if (list == nullptr) {
       list = create_user_handler_list<Event>();
     }

     return list->add(std::forward<typename Event::handler_func>(handler));
  }

  static void remove_all();

  template<typename Event>
  static void remove_all_for_event() {

    auto list = get_user_handler_list<Event>();
    if (!list) {
      return;
    }

    list->unregister_all();
  }

  template<typename Event>
  static bool remove_one_for_event(handler_id id) {
    auto list = get_user_handler_list<Event>();
    if (!list) {
      SYMPHONY_ALOG ("Invalid event handler id %d\n", id);
      return false;
    }

    return list->unregister_handler(id);
  }

};

template<typename Event>
class user_handler_list : public user_handler_list_base {

public:

  typedef typename Event::handler_func  handler_func;

private:

  struct entry {
    handler_id id;
    handler_func func;

    entry() : id(0), func(nullptr) { }
    entry(handler_id i, handler_func f) : id(i), func(f) {}
  };

public:

  typedef typename std::list<entry>::iterator iterator;

  user_handler_list() :
    _entries() {
  }

  void erase(iterator it) {
    _entries.erase(it);
  }

  bool is_empty () {
    return _entries.empty();
  }

  handler_id add(handler_func&& func) {
    handler_id id;
    if(_entries.empty()) {
      id = 0;
    } else {
      id = _entries.back().id + 1;
    }
    _entries.emplace_back(id, std::forward<handler_func>(func));
    return id;
  }

  void event_fired(Event&& event) {
    event_context context;
    for (auto& handler : _entries) {
      handler.func(std::forward<Event>(event), context);
    }
  }

  bool unregister_handler(handler_id id) {
    auto last =_entries.end();
    for (auto it = _entries.begin(); it != last; ++it) {
      if ((*it).id == id) {
        _entries.erase(it);
        return true;
      }
    }
    SYMPHONY_ALOG ("Invalid event handler id.\n");
    return false;
  }

  void unregister_all()  {
    _entries.clear();
  }

private:

  std::list <entry> _entries;

  SYMPHONY_DELETE_METHOD(user_handler_list(user_handler_list const&));
  SYMPHONY_DELETE_METHOD(user_handler_list(user_handler_list&&));
  SYMPHONY_DELETE_METHOD(user_handler_list& operator=(user_handler_list const&));
  SYMPHONY_DELETE_METHOD(user_handler_list& operator=(user_handler_list&&));
};

};
};
};

