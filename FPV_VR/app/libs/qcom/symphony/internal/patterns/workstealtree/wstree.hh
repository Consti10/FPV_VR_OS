// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/runtime.hh>
#include <symphony/internal/patterns/workstealtree/wstnodepool.hh>
#include <symphony/internal/patterns/workstealtree/wstree_base.hh>

namespace symphony {
namespace internal {

template <typename T> class ws_tree;

template <typename T>
class ws_node : public ws_node_base {

public:
  typedef ws_node<T>        node_type;
  typedef ws_tree<T>        tree_type;

private:
  typedef ws_node<T>*                          node_type_pointer;
  typedef ws_tree<T>*                          tree_type_pointer;
  typedef std::atomic<node_type_pointer>       atomic_node_type_pointer;

  atomic_node_type_pointer _left;
  atomic_node_type_pointer _right;
  tree_type_pointer        _tree;
  T                        _value;

  ws_node(size_type first, size_type last, size_type progress,
          tree_type_pointer tree, const T& identity) :
    ws_node_base(first, last, progress),
    _left(nullptr),
    _right(nullptr),
    _tree(tree),
    _value(std::move(identity)){

    }

  friend class ws_tree<T>;
  friend class testing::pool_tests;

  SYMPHONY_DELETE_METHOD(ws_node(ws_node const&));
  SYMPHONY_DELETE_METHOD(ws_node(ws_node&&));
  SYMPHONY_DELETE_METHOD(ws_node& operator=(ws_node const&));
  SYMPHONY_DELETE_METHOD(ws_node& operator=(ws_node&&));

public:

  ws_node() :
    ws_node_base(),
    _left(nullptr),
    _right(nullptr),
    _tree(nullptr),
    _value(){

  }

  node_type* get_left(symphony::mem_order order = symphony::mem_order_seq_cst) const
  { return _left.load(order); }
  node_type* get_right(symphony::mem_order order = symphony::mem_order_seq_cst) const
  { return _right.load(order); }

  void set_left(node_type_pointer node,
                symphony::mem_order order = symphony::mem_order_seq_cst)
  { _left.store(node, order); }
  void set_right(node_type_pointer node,
                 symphony::mem_order order = symphony::mem_order_seq_cst)
  { _right.store(node, order); }

  tree_type_pointer get_tree() const {
    return reinterpret_cast<tree_type_pointer>(
        reinterpret_cast<intptr_t>(_tree) & (~NODE_COMPLETE_BIT));
  }

  void set_completed() {
    _tree = reinterpret_cast<tree_type_pointer>(
        reinterpret_cast<intptr_t>(_tree) | NODE_COMPLETE_BIT);
  }

  bool is_completed() {
    return
      (reinterpret_cast<intptr_t>(_tree) & NODE_COMPLETE_BIT)
        == NODE_COMPLETE_BIT;
  }

  inline const T& peek_value() const { return _value; }
  inline       T& peek_value()       { return _value; }
  inline void set_value(T& val)      { _value = val; }
  inline void set_value(T&& val)     { _value = std::move(val); }

  bool is_leaf() { return get_left(symphony::mem_order_relaxed) == nullptr; }

  size_type get_split_idx() const {
    return this->get_left()->get_first() - 1;
  }

  static bool try_own(node_type* n);

  static try_steal_result try_steal(node_type* n,
                                    const size_type blk_size,
                                    const T& init);

};

template<> class ws_tree<void>;

template <>
class ws_node<void> : public ws_node_base {

public:
  typedef ws_node<void>        node_type;
  typedef ws_tree<void>        tree_type;

private:
  typedef ws_node<void>*                          node_type_pointer;
  typedef ws_tree<void>*                          tree_type_pointer;
  typedef std::atomic<node_type_pointer>       atomic_node_type_pointer;

  atomic_node_type_pointer _left;
  atomic_node_type_pointer _right;
  tree_type_pointer        _tree;

  ws_node(size_type first, size_type last, size_type progress,
          tree_type_pointer tree) :
    ws_node_base(first, last, progress),
    _left(nullptr),
    _right(nullptr),
    _tree(tree){}

  friend class ws_tree<void>;
  friend class testing::pool_tests;

  SYMPHONY_DELETE_METHOD(ws_node(ws_node const&));
  SYMPHONY_DELETE_METHOD(ws_node(ws_node&&));
  SYMPHONY_DELETE_METHOD(ws_node& operator=(ws_node const&));
  SYMPHONY_DELETE_METHOD(ws_node& operator=(ws_node&&));

public:

  ws_node() :
    ws_node_base(),
    _left(nullptr),
    _right(nullptr),
    _tree(nullptr){}

  node_type* get_left(symphony::mem_order order = symphony::mem_order_seq_cst) const
  { return _left.load(order); }
  node_type* get_right(symphony::mem_order order = symphony::mem_order_seq_cst) const
  { return _right.load(order); }

  void set_left(node_type_pointer node,
                symphony::mem_order order = symphony::mem_order_seq_cst)
  { _left.store(node, order); }
  void set_right(node_type_pointer node,
                 symphony::mem_order order = symphony::mem_order_seq_cst)
  { _right.store(node, order); }

  tree_type_pointer get_tree() const {
    return reinterpret_cast<tree_type_pointer>(
        reinterpret_cast<intptr_t>(_tree) & (~NODE_COMPLETE_BIT));
  }

  void set_completed() {
    _tree = reinterpret_cast<tree_type_pointer>(
        reinterpret_cast<intptr_t>(_tree) | NODE_COMPLETE_BIT);
  }

  bool is_completed() {
    return
      (reinterpret_cast<intptr_t>(_tree) & NODE_COMPLETE_BIT)
        == NODE_COMPLETE_BIT;
  }

  bool is_leaf() { return get_left(symphony::mem_order_relaxed) == nullptr; }

  static bool try_own(node_type* n);

  static try_steal_result try_steal(node_type* n, const size_type blk_size);

};

template <typename T>
class ws_tree : public ws_tree_base {
public:
  typedef ws_node<T>                node_type;

  typedef node_pool<node_type, 256> pool_type;

  ws_tree(size_t first, size_t last, size_t blk_size, const T& identity,
          size_t doc = internal::num_execution_contexts()) :
    ws_tree_base(blk_size, doc),
    _node_pool(_max_tasks),

    _root(new (get_new_node_placement())
          node_type(first, last, first + blk_size, this, identity)){
  }

  node_type* get_root()     const { return _root; }
  size_type  range_start()  const { return _root->get_first(); }

  static node_type* create_claimed_node(size_type first,
                                        size_type last,
                                        node_type* parent,
                                        const T& identity) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");

    ws_tree<T>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type(first, last, first, tree, identity);
  }

  static node_type* create_unclaimed_node(size_type first,
                                          size_type last,
                                          node_type* parent,
                                          const T& identity) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");

    ws_tree<T>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type
      (first, last, node_type::UNCLAIMED, tree, identity);
  }

  static node_type* create_stolen_node(size_type first,
                                       size_type last,
                                       node_type* parent,
                                       const T& identity) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");
    SYMPHONY_INTERNAL_ASSERT(first != node_type::UNCLAIMED,
                         "Stolen node can't be unclaimed");

    parent->inc_right_traversal();

    ws_tree<T>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type(first, last, first, tree, identity);
  }

  void split_tree_before_stealing(size_type nctx, const T& init);

  void split_tree_before_stealing(size_type nctx);

  node_type* find_work_prealloc(size_type tid);

  node_type* find_work_intree(node_type* n, size_type blk_size, const T& init);

  node_type* find_work_intree(size_type tid, node_type* n, size_type blk_size);

private:
  pool_type       _node_pool;
  node_type*      _root;

  node_type* get_new_node_placement() {
    return _node_pool.get_next();
  }

  SYMPHONY_DELETE_METHOD(ws_tree(ws_tree const&));
  SYMPHONY_DELETE_METHOD(ws_tree(ws_tree&&));
  SYMPHONY_DELETE_METHOD(ws_tree& operator=(ws_tree const&));
  SYMPHONY_DELETE_METHOD(ws_tree& operator=(ws_tree&&));
};

template <>
class ws_tree<void> : public ws_tree_base {
public:
  typedef ws_node<void>             node_type;
  typedef node_pool<node_type, 256> pool_type;

  ws_tree(size_t first, size_t last, size_t blk_size, size_t stride,
          size_t doc = internal::num_execution_contexts()) :
    ws_tree_base(blk_size, doc),
    _node_pool(_max_tasks),
    _root(new (get_new_node_placement())
          node_type(first, last, first + blk_size, this)),
    _stride(stride)
  {}

  node_type* get_root()    const { return _root; }
  size_type  range_start() const { return _root->get_first(); }
  size_type  get_stride()  const { return _stride; }

  static node_type* create_claimed_node(size_type first,
                                        size_type last,
                                        node_type* parent) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");

    ws_tree<void>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type(first, last, first, tree);
  }

  static node_type* create_unclaimed_node(size_type first, size_type last,
                                          node_type* parent) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");

    ws_tree<void>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type(first, last, node_type::UNCLAIMED, tree);
  }

  static node_type* create_stolen_node(size_type first, size_type last,
                                       node_type* parent) {
    SYMPHONY_INTERNAL_ASSERT(parent != nullptr, "Invalid parent pointer.");
    SYMPHONY_INTERNAL_ASSERT(first != node_type::UNCLAIMED,
                         "Stolen node can't be unclaimed");

    parent->inc_right_traversal();

    ws_tree<void>* tree = parent->get_tree();
    SYMPHONY_INTERNAL_ASSERT(tree != nullptr, "Invalid tree pointer.");
    auto* placement = tree->get_new_node_placement();
    return new (placement) node_type(first, last, first, tree);
  }

  void split_tree_before_stealing(size_type nctx);

  node_type* find_work_prealloc(size_type tid);

  node_type* find_work_intree(node_type* n, size_type blk_size);

private:
  pool_type       _node_pool;
  node_type*      _root;

  size_type       _stride;

  node_type* get_new_node_placement() {
    return _node_pool.get_next();
  }

  SYMPHONY_DELETE_METHOD(ws_tree(ws_tree const&));
  SYMPHONY_DELETE_METHOD(ws_tree(ws_tree&&));
  SYMPHONY_DELETE_METHOD(ws_tree& operator=(ws_tree const&));
  SYMPHONY_DELETE_METHOD(ws_tree& operator=(ws_tree&&));
};

#include <symphony/internal/patterns/workstealtree/wstree_impl.hh>

};
};
