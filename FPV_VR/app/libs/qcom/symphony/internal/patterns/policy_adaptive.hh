// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/workstealtree/wstree.hh>

namespace symphony {

namespace internal {

enum class Policy : uint8_t {
  REDUCE = 0,
  MAP    = 1,
  SCAN   = 2,
};

template <typename Fn>
class adaptive_strategy_base
{
public:
  typedef typename function_traits<Fn>::type_in_task f_type;

  adaptive_strategy_base(group_ptr g,
                         Fn&& f,
                         legacy::task_attrs attrs) :
    _group(g),
    _f(f),
    _task_attrs(attrs),
    _prealloc(false) {

  }

  group_ptr get_group() { return _group; }
  Fn& get_fn() { return _f; }
  bool is_prealloc() { return _prealloc; }
  legacy::task_attrs get_task_attrs() const { return _task_attrs;}

protected:
  group_ptr                   _group;
  f_type                      _f;
  legacy::task_attrs          _task_attrs;

  bool                        _prealloc;

private:

  SYMPHONY_DELETE_METHOD(adaptive_strategy_base(adaptive_strategy_base const&));
  SYMPHONY_DELETE_METHOD(adaptive_strategy_base(adaptive_strategy_base&&));
  SYMPHONY_DELETE_METHOD(adaptive_strategy_base&
      operator=(adaptive_strategy_base const&));
  SYMPHONY_DELETE_METHOD(adaptive_strategy_base&
      operator=(adaptive_strategy_base&&));
};

template <typename T>
class tree_ops_base
{
public:
  typedef size_t     size_type;
  typedef ws_tree<T> tree_type;
  typedef ws_node<T> work_item_type;

  template<typename SizeType,
           typename U,
           typename = typename std::enable_if
             <!std::is_same<U, void>::value>::type>
  tree_ops_base(SizeType first,
                SizeType last,
                SizeType blk_size,
                const U& identity,
                const symphony::pattern::tuner& tuner) :
    _workstealtree(first, last, blk_size, identity, tuner.get_doc()){

    }

  template<typename SizeType,
           typename U,
           typename = typename std::enable_if
             <std::is_same<U, void>::value>::type>
  tree_ops_base(SizeType first,
                SizeType last,
                SizeType blk_size,
                SizeType stride,
                const symphony::pattern::tuner& tuner) :
    _workstealtree(first, last, blk_size, stride, tuner.get_doc()){

    }

  tree_ops_base(size_t first,
                size_t last,
                size_t blk_size,
                const symphony::pattern::tuner& tuner) :
    _workstealtree(first, last, blk_size, T(), tuner.get_doc()){

    }

  size_type get_max_tasks() const { return _workstealtree.get_max_tasks(); }
  size_type get_blk_size()  const { return _workstealtree.get_blk_size(); }

  work_item_type* find_work_prealloc(size_type task_id)
  {
    return _workstealtree.find_work_prealloc(task_id);
  }

  work_item_type* get_root() const { return _workstealtree.get_root(); }
  size_type get_prealloc_leaf() { return _workstealtree.get_leaf_num(); }

#ifdef SYMPHONY_ADAPTIVE_PFOR_DEBUG
  const tree_type& get_tree() const { return _workstealtree; }
#endif

protected:
  tree_type _workstealtree;

private:

  SYMPHONY_DELETE_METHOD(tree_ops_base(tree_ops_base const&));
  SYMPHONY_DELETE_METHOD(tree_ops_base(tree_ops_base&&));
  SYMPHONY_DELETE_METHOD(tree_ops_base& operator=(tree_ops_base const&));
  SYMPHONY_DELETE_METHOD(tree_ops_base& operator=(tree_ops_base&&));
};

template<typename T,
         typename InputIterator,
         typename Fn,
         typename BinaryFn,
         Policy P>
class adaptive_steal_strategy;

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename T, typename Fn>
class adaptive_steal_strategy<T, size_t, Fn, Fn, Policy::REDUCE>:
  public adaptive_strategy_base<Fn>,
  public tree_ops_base<T>,
  public ref_counted_object<adaptive_steal_strategy
                            <T, size_t, Fn, Fn, Policy::REDUCE>>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");
public:
  typedef size_t                                       size_type;
  typedef ws_tree<T>                                   tree_type;
  typedef ws_node<T>                                   work_item_type;

  adaptive_steal_strategy(group_ptr g,
                          size_type first,
                          size_type last,
                          const T& identity,
                          Fn&& f,
                          legacy::task_attrs attrs,
                          size_type blk_size,
                          const symphony::pattern::tuner& tuner) :
    adaptive_strategy_base<Fn>(g, std::forward<Fn>(f), attrs),
    tree_ops_base<T>(first, last, blk_size, identity, tuner),
    _identity(std::move(identity)){

    }

  void static_split(size_type max_tasks, const T& identity) {
    tree_ops_base<T>::_workstealtree.split_tree_before_stealing(max_tasks, identity);
    adaptive_strategy_base<Fn>::_prealloc = true;
  }

  work_item_type* find_work_intree
    (work_item_type* n, size_type blk_size, const T& identity)
  {
    return tree_ops_base<T>::_workstealtree.find_work_intree(n, blk_size, identity);
  }

 const T& get_identity() const { return _identity; }

private:
  const T _identity;

  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy&&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy&&));

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename T, typename InputIterator, typename Fn, typename BinaryFn>
class adaptive_steal_strategy<T, InputIterator, Fn, BinaryFn, Policy::SCAN>:
  public adaptive_strategy_base<Fn>,
  public tree_ops_base<T>,
  public ref_counted_object<adaptive_steal_strategy
                            <T, InputIterator, Fn, BinaryFn, Policy::SCAN>>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");
public:
  typedef size_t                size_type;
  typedef ws_tree<T>            tree_type;
  typedef InputIterator         iter_type;
  typedef ws_node<T>            work_item_type;
  typedef uint8_t               counter_type;
  typedef std::atomic<uint8_t>  atomic_counter_type;
  typedef std::atomic<bool>     atomic_bool_type;

  adaptive_steal_strategy(group_ptr g,
                          InputIterator base,
                          size_type first,
                          size_type last,
                          Fn&& f,
                          BinaryFn&& bf,
                          legacy::task_attrs attrs,
                          size_type blk_size,
                          const symphony::pattern::tuner& tuner) :
    adaptive_strategy_base<Fn>(g, std::forward<Fn>(f), attrs),
    tree_ops_base<T>(first, last, blk_size, tuner),
    _bf(bf),
    _base(base),
    _task_counter(0),
    _lsum_update_done(false){

    }

  BinaryFn& get_binary_fn() { return _bf; }
  iter_type get_base() { return _base; }

  void static_split(size_type max_tasks) {
    tree_ops_base<T>::_workstealtree.split_tree_before_stealing(max_tasks);
    adaptive_strategy_base<Fn>::_prealloc = true;
  }

  work_item_type* find_work_intree(size_type tid, work_item_type* n, size_type blk_size)
  {
    return tree_ops_base<T>::_workstealtree.find_work_intree(tid, n, blk_size);
  }

  void inc_task_counter(symphony::mem_order order = symphony::mem_order_seq_cst) {
    _task_counter.fetch_add(1, order);
  }

  uint8_t get_task_counter(symphony::mem_order order = symphony::mem_order_seq_cst) {
    return _task_counter.load(order);
  }

  void set_local_sum_updated(symphony::mem_order order = symphony::mem_order_seq_cst) {
    _lsum_update_done.store(true, order);
  }

  bool is_local_sum_updated(symphony::mem_order order = symphony::mem_order_seq_cst) {
    return _lsum_update_done.load(order);
  }

private:
  BinaryFn             _bf;
  iter_type            _base;

  atomic_counter_type  _task_counter;

  atomic_bool_type     _lsum_update_done;

  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy&&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy&&));

};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename Fn>
class adaptive_steal_strategy<void, size_t, Fn, Fn, Policy::MAP> :
  public adaptive_strategy_base<Fn>,
  public tree_ops_base<void>,
  public ref_counted_object<adaptive_steal_strategy<void, size_t, Fn, Fn, Policy::MAP>>
{
SYMPHONY_GCC_IGNORE_END("-Weffc++");

public:
  typedef size_t                                       size_type;
  typedef ws_tree<void>                                tree_type;
  typedef ws_node<void>                                work_item_type;

  adaptive_steal_strategy(group_ptr g,
                          size_type first,
                          size_type last,
                          Fn&& f,
                          legacy::task_attrs attrs,
                          size_type blk_size,
                          size_type stride,
                          const symphony::pattern::tuner& tuner) :
    adaptive_strategy_base<Fn>(g, std::forward<Fn>(f), attrs),
    tree_ops_base<void>(first, last, blk_size, stride, tuner){

    }

  size_type get_stride()    const { return _workstealtree.get_stride(); }

  void static_split(size_type max_tasks) {
    _workstealtree.split_tree_before_stealing(max_tasks);
    adaptive_strategy_base<Fn>::_prealloc = true;
  }

  work_item_type* find_work_intree(work_item_type* n, size_type blk_size)
  {
    return _workstealtree.find_work_intree(n, blk_size);
  }

private:

  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy
      (adaptive_steal_strategy&&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy const&));
  SYMPHONY_DELETE_METHOD(adaptive_steal_strategy&
      operator=(adaptive_steal_strategy&&));

};

};
};
