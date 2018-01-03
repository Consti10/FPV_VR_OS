// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <cstdlib>
#include <symphony/internal/util/memorder.hh>

namespace symphony {
namespace internal {

enum class try_steal_result : uint8_t {
  SUCCESS          = 0,
  ALREADY_FINISHED = 1,
  ALREADY_STOLEN   = 2,
  NULL_POINTER     = 3,
  INVALID          = 4,
};

class ws_node_base {

public:
  typedef uint16_t               counter_type;
  typedef size_t                 size_type;

  static const size_type         UNCLAIMED;
  static const size_type         STOLEN;
  static const intptr_t          NODE_COMPLETE_BIT;

protected:
  typedef std::atomic<size_type> atomic_size_type;

#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
  atomic_size_type _traversal;
  size_type        _worker_id;
  size_type        _progress_save;
#endif

  counter_type             _left_traversal;
  counter_type             _right_traversal;
  const size_type          _first;
  const size_type          _last;
  atomic_size_type         _progress;
  bool                     _pre_assigned;

  SYMPHONY_DELETE_METHOD(ws_node_base(ws_node_base const&));
  SYMPHONY_DELETE_METHOD(ws_node_base(ws_node_base&&));
  SYMPHONY_DELETE_METHOD(ws_node_base& operator=(ws_node_base const&));
  SYMPHONY_DELETE_METHOD(ws_node_base& operator=(ws_node_base&&));

public:

  ws_node_base() :
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
    _traversal(0),
    _worker_id(0),
    _progress_save(0),
#endif
    _left_traversal(0),
    _right_traversal(0),
    _first(0),
    _last(0),
    _progress(0),
    _pre_assigned(false){}

  ws_node_base(size_type first, size_type last, size_type progress) :
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
    _traversal(0),
    _worker_id(0),
    _progress_save(0),
#endif
    _left_traversal(0),
    _right_traversal(0),
    _first(first),
    _last(last),
    _progress(progress),
    _pre_assigned(false){}

  virtual ~ws_node_base(){}

#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
  size_type count_traversal() const {
    return _traversal.load(symphony::mem_order_relaxed);
  }
  void increase_traversal(size_type n,
                          symphony::mem_order order = symphony::mem_order_seq_cst)
  {
    _traversal.fetch_add(n, order);
  }

  size_type get_worker_id() const { return _worker_id; }
  void set_worker_id(size_type id){ _worker_id = id; }

  size_type get_progress_save() const { return _progress_save; }
  void set_progress_save(size_type psave) { _progress_save = psave; }
#endif

  bool is_unclaimed(symphony::mem_order order = symphony::mem_order_seq_cst) {
    return get_progress(order) == UNCLAIMED;
  }

  bool is_stolen(size_type blk_size,
                 symphony::mem_order order = symphony::mem_order_seq_cst) {
    auto progress = get_progress(order);
    return (progress == STOLEN) || (progress == STOLEN + blk_size);
  }

  size_type get_first() const { return _first; }
  size_type get_last() const { return _last; }

  bool is_assigned() const { return _pre_assigned; }
  void set_assigned() { _pre_assigned = true; }

  size_type get_progress(symphony::mem_order order = symphony::mem_order_seq_cst) {
    return _progress.load(order);
  }

  size_type inc_progress(size_type n, symphony::mem_order order) {
    return _progress.fetch_add(n, order);
  }

  size_type get_left_traversal() const { return _left_traversal; }
  size_type get_right_traversal() const { return _right_traversal; }

  void save_task_id(size_t id) {
    _left_traversal += static_cast<counter_type>(id << 12);
  }

  counter_type get_task_id() const {
    return _left_traversal >> 12;
  }

  void inc_left_traversal()  { _left_traversal++; }
  void inc_right_traversal() { _right_traversal++; }

};

class ws_tree_base {

public:
  typedef size_t size_type;

  explicit ws_tree_base(size_type blk_size, size_type doc) :
    _max_tasks(doc),
    _prealloc_leaf(1),
    _prealloc_level(0),
    _blk_size(blk_size) {}

  virtual ~ws_tree_base(){}

  size_type  get_max_tasks() const { return _max_tasks; }
  size_type  get_leaf_num() const { return _prealloc_leaf; }
  size_type  get_level() const { return _prealloc_level; }
  size_type  get_blk_size() const { return _blk_size; }

protected:

  const size_type _max_tasks;
  size_type       _prealloc_leaf;
  size_type       _prealloc_level;
  size_type       _blk_size;

  SYMPHONY_DELETE_METHOD(ws_tree_base(ws_tree_base const&));
  SYMPHONY_DELETE_METHOD(ws_tree_base(ws_tree_base&&));
  SYMPHONY_DELETE_METHOD(ws_tree_base& operator=(ws_tree_base const&));
  SYMPHONY_DELETE_METHOD(ws_tree_base& operator=(ws_tree_base&&));
};

};
};

