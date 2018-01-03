// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/log/log.hh>

namespace symphony {

namespace internal {

namespace testing {
  class pool_tests;
};

template<typename T, size_t Size_>
class node_pool {

public:

  typedef T          node_type;
  typedef size_t     size_type;
  typedef node_type* pointer;

private:

  class node_slab {

    node_type _buffer[Size_];
    const size_type _first_pos;
    std::atomic<node_slab*> _next;

  public:

    explicit node_slab(size_type first_pos):
      _first_pos(first_pos),
      _next(nullptr) {
    }

    size_type begin() const { return _first_pos; }
    size_type end() const { return begin() + Size_; }

    bool has(size_type pos) const {
      return ((pos >= begin()) && (pos < end()));
    }

    void set_next(node_slab* next, symphony::mem_order order) {
      _next.store(next, order);
    }

    node_slab* get_next(symphony::mem_order order) {
      return _next.load(order);
    }

    pointer get_ptr(size_type pos) {
      return &_buffer[pos];
    }

    SYMPHONY_DELETE_METHOD(node_slab(node_slab const&));
    SYMPHONY_DELETE_METHOD(node_slab(node_slab&&));
    SYMPHONY_DELETE_METHOD(node_slab& operator=(node_slab const&));
    SYMPHONY_DELETE_METHOD(node_slab& operator=(node_slab&&));

    ~node_slab() {}
  };

  node_slab _inlined_slab;
  std::atomic<size_type> _pos;
  std::atomic<node_slab*> _slab;

  node_slab* find_slab(const size_type pos, node_slab* first_slab) {
    auto ck = first_slab;
    while(ck != nullptr) {
      if (ck->has(pos))
        return ck;
      ck = ck->get_next(symphony::mem_order_acquire);
    }
    return nullptr;
  }

  node_slab* grow(size_type pos, node_slab* current_slab) {

    SYMPHONY_INTERNAL_ASSERT(current_slab, "null ptr");

    if (pos == 0)
      return get_inlined_slab();

    node_slab* new_slab = new node_slab(pos);

    size_type current_end = current_slab->end();
    if (current_end != pos) {
      current_slab = find_slab(pos - 1, get_inlined_slab());
      SYMPHONY_INTERNAL_ASSERT(current_slab != nullptr, "null ptr");
    }

    SYMPHONY_INTERNAL_ASSERT(current_slab->has(pos - 1), "Wrong slab");

    current_slab->set_next(new_slab, symphony::mem_order_relaxed);

    _slab.store(new_slab, symphony::mem_order_release);
    log::fire_event<log::events::ws_tree_new_slab>();
    return new_slab;
  }

  pointer get_next_impl() {

    auto pos = _pos.fetch_add(size_type(1), symphony::mem_order_relaxed);
    auto slab = _slab.load(symphony::mem_order_relaxed);

    const auto pos_in_slab = pos % Size_;

    SYMPHONY_INTERNAL_ASSERT(slab != nullptr, "Invalid ptr null");

    if (slab->has(pos))
      return slab->get_ptr(pos_in_slab);

    slab = _slab.load(symphony::mem_order_acquire);

    if (pos_in_slab == 0){

      return grow(pos, slab)->get_ptr(0);
    }

    do {
      slab = find_slab(pos, get_inlined_slab());
    } while (!slab);

    SYMPHONY_INTERNAL_ASSERT(slab->has(pos), "Wrong slab");

    return slab->get_ptr(pos_in_slab);
  }

public:

  explicit node_pool(size_type max_sharers) :
    _inlined_slab(0),
    _pos(0),
    _slab(&_inlined_slab) {
    SYMPHONY_INTERNAL_ASSERT(max_sharers < Size_, "Too many sharers for pool.");
  }

  pointer get_next() {
    auto next = get_next_impl();
    SYMPHONY_API_ASSERT(next, "Could not allocate slab.");
    log::fire_event<log::events::ws_tree_node_created>();
    return next;
  }

  node_slab* get_inlined_slab() { return &_inlined_slab; }

  ~node_pool() {
    auto next_slab = _inlined_slab.get_next(symphony::mem_order_acquire);
    while (next_slab) {
      auto tmp = next_slab->get_next(symphony::mem_order_relaxed);
      delete next_slab;
      next_slab = tmp;
    }
  };

  SYMPHONY_DELETE_METHOD(node_pool(node_pool const&));
  SYMPHONY_DELETE_METHOD(node_pool(node_pool&&));
  SYMPHONY_DELETE_METHOD(node_pool& operator=(node_pool const&));
  SYMPHONY_DELETE_METHOD(node_pool& operator=(node_pool&&));
};

};
};
