// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

template<typename T>
bool ws_node<T>::try_own(node_type* n)
{

  SYMPHONY_INTERNAL_ASSERT(n != nullptr, "Call try_own from a nullptr!");

  log::fire_event<log::events::ws_tree_worker_try_own>();

  if (n->_progress.load(symphony::mem_order_relaxed) != UNCLAIMED)
    return false;

  size_type unclaimed = UNCLAIMED;
  return (std::atomic_compare_exchange_strong( &(n->_progress),
                                               &unclaimed,
                                               n->_first) );
}

template<typename T>
try_steal_result ws_node<T>::try_steal(node_type* n,
                                       const size_type blk_size,
                                       const T& init)
{

  SYMPHONY_INTERNAL_ASSERT(n != nullptr, "Call try_steal from a nullptr!");

  log::fire_event<log::events::ws_tree_worker_try_steal>();

  size_type iter;
  const size_type stolen = STOLEN;
  bool success = false;

  do {

    iter = n->_progress.load(symphony::mem_order_relaxed);
    SYMPHONY_INTERNAL_ASSERT(iter != UNCLAIMED, "Can't steal an unclaimed node");

    if (iter == stolen || iter == blk_size - 1)
      return try_steal_result::ALREADY_STOLEN;

    if (n->_last == 0)
      return try_steal_result::ALREADY_FINISHED;

    if ((iter + blk_size) >= n->_last)
      return try_steal_result::ALREADY_FINISHED;

    success = std::atomic_compare_exchange_weak_explicit( &(n->_progress),
                                                 &iter,
                                                 stolen,
                                                 symphony::mem_order_acq_rel,
                                                 symphony::mem_order_relaxed);
  } while(!success);

#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG

    n->_progress_save = iter - n->_first;
#endif

   size_type start = iter + blk_size;
   size_type mid = start + (n->_last - start) / 2;

   n->_left  = tree_type::create_unclaimed_node(start, mid, n, init);
   n->_right = tree_type::create_stolen_node(mid + 1, n->_last, n, init);

   return try_steal_result::SUCCESS;

}

template <typename T>
void ws_tree<T>::split_tree_before_stealing(size_type nctx, const T& init)
{
  SYMPHONY_INTERNAL_ASSERT(_root != nullptr, "Invalid root to split");

  size_type level_start_pos = 0;

  while(nctx != 1) {
    size_type pos = level_start_pos;

    for(size_type i = 0; i < _prealloc_leaf; ++i, ++pos){

      node_type* n = _node_pool.get_inlined_slab()->get_ptr(pos);
      size_type start = n->get_first();
      size_type mid = start + (n->get_last() - start) / 2;
      n->set_left(create_claimed_node(start, mid, n, init),
                  symphony::mem_order_relaxed);
      n->set_right(create_claimed_node(mid + 1, n->get_last(), n, init),
                   symphony::mem_order_relaxed);

      if (nctx < 4) {
        n->get_left(symphony::mem_order_relaxed)->set_assigned();
        n->get_right(symphony::mem_order_relaxed)->set_assigned();

        if (i == 0) {
          n->get_left(symphony::mem_order_relaxed)->
                        inc_progress(_blk_size, symphony::mem_order_relaxed);
        }
      }
    }

    nctx >>= 1;
    level_start_pos += _prealloc_leaf;
    _prealloc_leaf <<= 1;
    _prealloc_level++;
  }
}

template <typename T>
void ws_tree<T>::split_tree_before_stealing(size_type nctx)
{
  SYMPHONY_INTERNAL_ASSERT(_root != nullptr, "Invalid root to split");

  size_type level_start_pos = 0;
  T init = T();

  while(nctx != 1) {
    size_type pos = level_start_pos;

    for(size_type i = 0; i < _prealloc_leaf; ++i, ++pos){

      node_type* n = _node_pool.get_inlined_slab()->get_ptr(pos);
      size_type start = n->get_first();
      size_type mid = start + (n->get_last() - start) / 2;
      n->set_left(create_claimed_node(start, mid, n, init),
                  symphony::mem_order_relaxed);
      n->set_right(create_claimed_node(mid + 1, n->get_last(), n, init),
                   symphony::mem_order_relaxed);

      if (nctx < 4 && i == 0) {
        n->get_left(symphony::mem_order_relaxed)->inc_progress(_blk_size, symphony::mem_order_relaxed);
      }
    }

    nctx >>= 1;
    level_start_pos += _prealloc_leaf;
    _prealloc_leaf <<= 1;
    _prealloc_level++;
  }

}
template <typename T>
typename ws_tree<T>::node_type* ws_tree<T>::find_work_prealloc(size_type tid)
{

  node_type* n = _root;
  size_type temp = tid;

  for (size_type i = 0; i < _prealloc_level; ++i, temp >>= 1) {
    SYMPHONY_INTERNAL_ASSERT(n != nullptr, "Fail to pre-allocate tree");
    if (!(temp & 0x1))
      n = n->get_left(symphony::mem_order_relaxed);
    else
      n = n->get_right(symphony::mem_order_relaxed);
  }

  return n;

}

template <typename T>
typename ws_tree<T>::node_type*
ws_tree<T>::find_work_intree(node_type* n,
                             size_type blk_size,
                             const T& init)
{

  if (n == nullptr)
    return nullptr;

  if (n->is_completed())
    return nullptr;

  if (n->is_leaf() && !n->is_assigned()) {
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
    n->increase_traversal(1, symphony::mem_order_relaxed);
#endif

    if (node_type::try_own(n)) {
      log::fire_event<log::events::ws_tree_try_own_success>();
      return n;
    }

    auto steal_result = node_type::try_steal(n, blk_size, init);

    if (steal_result == try_steal_result::SUCCESS) {
      log::fire_event<log::events::ws_tree_try_steal_success>();
      return n->get_right(symphony::mem_order_relaxed);
    }

    else if (steal_result == try_steal_result::ALREADY_FINISHED) {
      return nullptr;
    }

    SYMPHONY_INTERNAL_ASSERT(steal_result == try_steal_result::ALREADY_STOLEN,
                         "Invalid node state.");
  }

  node_type* first;
  node_type* second;

  if (n->get_left_traversal() <= n->get_right_traversal()) {
    n->inc_left_traversal();
    first = n->get_left(symphony::mem_order_relaxed);
    second = n->get_right(symphony::mem_order_relaxed);
  } else {
    n->inc_right_traversal();
    first = n->get_right(symphony::mem_order_relaxed);
    second = n->get_left(symphony::mem_order_relaxed);
  }

  n = find_work_intree(first, blk_size, init);
  if (n != nullptr)
    return n;
  return find_work_intree(second, blk_size, init);

}

template <typename T>
typename ws_tree<T>::node_type*
ws_tree<T>::find_work_intree(size_type task_id,
                             node_type* n,
                             size_type blk_size)
{

  if (n == nullptr)
    return nullptr;

  if (n->is_completed())
    return nullptr;

  if (n->is_leaf()) {
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
    n->increase_traversal(1, symphony::mem_order_relaxed);
#endif

    if (node_type::try_own(n)) {
      n->save_task_id(task_id);
      log::fire_event<log::events::ws_tree_try_own_success>();
      return n;
    }

    T init = T();
    auto steal_result = node_type::try_steal(n, blk_size, init);

    if (steal_result == try_steal_result::SUCCESS) {
      n->get_right()->save_task_id(task_id);
      log::fire_event<log::events::ws_tree_try_steal_success>();
      return n->get_right(symphony::mem_order_relaxed);
    }

    else if (steal_result == try_steal_result::ALREADY_FINISHED) {
      return nullptr;
    }

    SYMPHONY_INTERNAL_ASSERT(steal_result == try_steal_result::ALREADY_STOLEN,
                         "Invalid node state.");
  }

  node_type* first;
  node_type* second;

  uint16_t mask = 0x0FFF;
  uint16_t ltraversal = n->get_left_traversal() & mask;
  if (ltraversal <= n->get_right_traversal()) {
    n->inc_left_traversal();
    first = n->get_left(symphony::mem_order_relaxed);
    second = n->get_right(symphony::mem_order_relaxed);
  } else {
    n->inc_right_traversal();
    first = n->get_right(symphony::mem_order_relaxed);
    second = n->get_left(symphony::mem_order_relaxed);
  }

  n = find_work_intree(task_id, first, blk_size);
  if (n != nullptr)
    return n;
  return find_work_intree(task_id, second, blk_size);

}
