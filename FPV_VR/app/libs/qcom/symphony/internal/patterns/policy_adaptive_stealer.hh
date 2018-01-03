// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/policy_adaptive_worker.hh>

namespace symphony {

namespace internal {

template<typename T, typename Strategy, Policy P>
struct stealer_wrapper;

template<typename T, typename Strategy>
struct stealer_wrapper<T, Strategy, Policy::REDUCE>
{
  static void stealer_task_body(symphony_shared_ptr<Strategy> stg_ptr, size_t task_id){
    SYMPHONY_INTERNAL_ASSERT(current_task() != nullptr,
        "current task pointer unexpectedly invalid");

    auto  root      = stg_ptr->get_root();
    auto  curr_node = root;
    auto& fn        = stg_ptr->get_fn();
    auto  blk_size  = stg_ptr->get_blk_size();

    if (stg_ptr->is_prealloc() && task_id < stg_ptr->get_prealloc_leaf()) {
      curr_node = stg_ptr->find_work_prealloc(task_id);
    }
    else{

      if (task_id != 0) {
        curr_node = stg_ptr->find_work_intree
          (root, blk_size, stg_ptr->get_identity());
      }
    }

    if (curr_node == nullptr)
      return ;

    do {
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG

      curr_node->set_worker_id(task_id);
#endif
      bool work_complete = worker_wrapper<T, decltype(fn), Policy::REDUCE>
                            ::work_on(curr_node, fn);
      if (work_complete)
        curr_node = stg_ptr->find_work_intree
          (root, blk_size, stg_ptr->get_identity());

      else
        curr_node = stg_ptr->find_work_intree
          (curr_node, blk_size, stg_ptr->get_identity());
    } while(curr_node != nullptr);
  }
};

template<typename T, typename Strategy>
struct stealer_wrapper<T, Strategy, Policy::SCAN>
{

  template<typename InputIterator, typename BinaryFn>
  static void update_local_sum(typename ws_node<T>::node_type* n,
                               const InputIterator base,
                               BinaryFn&& fn)
  {
    if (n == nullptr)
      return ;

    auto first = n->get_first();
    auto last =  n->get_last();
    T    val;

    if (first != n->get_tree()->range_start()) {
      val = *(base + first - 1);

      n->set_value(val);
      if (n->get_left() != 0) {
        auto offset = n->get_split_idx();
        if (offset >= first) {
          *(base + offset) = fn(val, *(base + offset));
        }
      }
      else{
        *(base + last) = fn(val, *(base + last));
      }
    }

    update_local_sum<InputIterator, BinaryFn>(n->get_left(), base, fn);
    update_local_sum<InputIterator, BinaryFn>(n->get_right(), base, fn);
  }

  template<typename InputIterator, typename BinaryFn>
  static void scan_node(typename ws_node<T>::node_type* n,
                        size_t tid,
                        const InputIterator base,
                        BinaryFn&& fn)
  {

    if (n == nullptr)
      return ;

    auto first = n->get_first();
    auto last  = n->get_last();

    if (first != 0) {

      if ( n->get_task_id() == static_cast<uint16_t>(tid)) {

        if (n->get_left() == nullptr) {
          for (auto idx = last - 1; idx >= first; --idx){
            *(base + idx) = fn(n->peek_value(), *(base + idx));
          }
        }

        else {
          auto offset = n->get_split_idx();
          if ((offset - 1) >= first) {
            for (auto idx = offset - 1; idx >= first; --idx) {
              *(base + idx) = fn(n->peek_value(), *(base + idx));
            }
          }
        }

      }
    }

    scan_node<InputIterator, BinaryFn>(n->get_left(), tid, base, fn);
    scan_node<InputIterator, BinaryFn>(n->get_right(), tid, base, fn);
  }

  static void stealer_task_body(symphony_shared_ptr<Strategy> stg_ptr,
                                size_t task_id)
  {
    SYMPHONY_INTERNAL_ASSERT(current_task() != nullptr,
        "current task pointer unexpectedly invalid");

    auto  root      = stg_ptr->get_root();
    auto  curr_node = root;
    auto& fn        = stg_ptr->get_fn();
    auto& bf        = stg_ptr->get_binary_fn();
    auto  blk_size  = stg_ptr->get_blk_size();
    auto  base      = stg_ptr->get_base();

    if (stg_ptr->is_prealloc() && task_id < stg_ptr->get_prealloc_leaf()) {
      curr_node = stg_ptr->find_work_prealloc(task_id);
    }
    else{

      if (task_id != 0) {
        curr_node = stg_ptr->find_work_intree(task_id, root, blk_size);
      }
    }

    if (curr_node == nullptr) {

      stg_ptr->inc_task_counter();
      return ;
    }

    do {
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG

      curr_node->set_worker_id(task_id);
#endif
      bool work_complete = worker_wrapper<T, decltype(fn), Policy::SCAN>
                            ::work_on(curr_node, fn);
      if (work_complete)
        curr_node = stg_ptr->find_work_intree(task_id, root, blk_size);

      else
        curr_node = stg_ptr->find_work_intree(task_id, curr_node, blk_size);
    } while(curr_node != nullptr);

    stg_ptr->inc_task_counter();

    if (task_id == 0) {

      while (stg_ptr->get_task_counter() !=
             stg_ptr->get_max_tasks())
      {}

      update_local_sum(root, base, bf);
      stg_ptr->set_local_sum_updated(symphony::mem_order_release);
    }

    while(!stg_ptr->is_local_sum_updated(symphony::mem_order_acquire)) {}

    scan_node(root, task_id, base, bf);
  }
};

template<typename Strategy>
struct stealer_wrapper<void, Strategy, Policy::MAP>
{
  static void stealer_task_body(symphony_shared_ptr<Strategy> stg_ptr, size_t task_id){
    SYMPHONY_INTERNAL_ASSERT(current_task() != nullptr,
        "current task pointer unexpectedly invalid");

    auto  root      = stg_ptr->get_root();
    auto  curr_node = root;
    auto& fn        = stg_ptr->get_fn();
    auto  blk_size  = stg_ptr->get_blk_size();

    if (stg_ptr->is_prealloc() && task_id < stg_ptr->get_prealloc_leaf()) {
      curr_node = stg_ptr->find_work_prealloc(task_id);
    }
    else{

      if (task_id != 0) {
        curr_node = stg_ptr->find_work_intree(root, blk_size);
      }
    }

    if (curr_node == nullptr)
      return ;

    do {
#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG

      curr_node->set_worker_id(task_id);
#endif
      bool work_complete = worker_wrapper<void, decltype(fn), Policy::MAP>
                            ::work_on(curr_node, fn);

      if (work_complete)
        curr_node = stg_ptr->find_work_intree(root, blk_size);

      else
        curr_node = stg_ptr->find_work_intree(curr_node, blk_size);
    } while(curr_node != nullptr);
  }
};

};
};
