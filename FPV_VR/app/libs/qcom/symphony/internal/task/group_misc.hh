// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <forward_list>

#include <symphony/internal/legacy/types.hh>
#include <symphony/internal/task/group_shared_ptr.hh>
#include <symphony/internal/task/group_signature.hh>
#include <symphony/internal/util/concurrent_dense_bitmap.hh>
#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {

namespace testing {
  class group_tester;
};

class group;
class meet;

namespace group_misc {

class lattice;
class lattice_node_leaf;

struct id {
  typedef size_t type;
};

class id_generator {

  static concurrent_dense_bitmap s_bmp;

public:

  static constexpr id::type default_meet_id = ~id::type(0);

  static size_t get_num_leaves() {
    return (s_bmp.popcount());
  }

  static id::type get_leaf_id() {
    return (s_bmp.set_first());
  }

  static void release_id(id::type id) {
    s_bmp.reset(id);
  }

  friend class testing::group_tester;
};

class meet_db {

  friend class lattice;
  friend class lattice_node_leaf;

  typedef std::forward_list<group*> meet_list;
private:
  explicit meet_db(size_t leaf_id):
    _leaf_id(leaf_id),
    _meet_list() {};

public:
  size_t get_leaf_id() const {return _leaf_id;};
  bool empty() {return _meet_list.empty();};

  group* find(group_signature& sig);

  void add(group* g) {
#ifdef SYMPHONY_DEBUG
    s_meet_groups_in_db++;
#endif
    _meet_list.push_front(g);
  }

  void remove(group* g) {
#ifdef SYMPHONY_DEBUG
    s_meet_groups_in_db--;
#endif
    _meet_list.remove(g);
  }

  size_t size() {
    return std::distance(_meet_list.begin(), _meet_list.end());
  }

  SYMPHONY_DELETE_METHOD(meet_db(meet_db const&));
  SYMPHONY_DELETE_METHOD(meet_db(meet_db&&));
  SYMPHONY_DELETE_METHOD(meet_db& operator=(meet_db const&));
  SYMPHONY_DELETE_METHOD(meet_db& operator=(meet_db&&));
  static std::atomic<size_t> s_meet_groups_in_db;

  static size_t meet_groups_in_db() {return s_meet_groups_in_db.load();};

private:

  size_t _leaf_id;

  meet_list _meet_list;
};

class lattice_node
{

  friend class ::symphony::internal::group;
  friend class ::symphony::internal::meet;

public:
  typedef std::forward_list<group*> children_list;
  typedef std::forward_list<group_shared_ptr> parent_list;

protected:

  static const size_t s_max_order = 200;

  group_signature _signature;

  parent_list _parents;
  children_list _children;

  size_t _order;

  group_misc::meet_db* _meet_db;

  bool _parent_sees_tasks;

protected:

  lattice_node(size_t id, size_t order,
      group_misc::meet_db* db):
  _signature(id, sparse_bitmap::singleton),
  _parents(),
  _children(),
  _order(order),
  _meet_db(db),
  _parent_sees_tasks(false) {
    SYMPHONY_API_ASSERT(_order <= s_max_order,
                    "Too many groups intersected. Max is %zu.",
                    s_max_order);
  }

  lattice_node(group_signature& signature, size_t order,
      group_misc::meet_db* db):
  _signature(std::move(signature)),
  _parents(),
  _children(),
  _order(order),
  _meet_db(db),
  _parent_sees_tasks(false) {
    SYMPHONY_API_ASSERT(_order <= s_max_order,
                    "Too many groups intersected. Max is %zu.",
                    s_max_order);
  }

  lattice_node(size_t order,
      group_misc::meet_db* db):
  _signature(),
  _parents(),
  _children(),
  _order(order),
  _meet_db(db),
  _parent_sees_tasks(false) {
    SYMPHONY_API_ASSERT(_order <= s_max_order,
                    "Too many groups intersected. Max is %zu.",
                    s_max_order);
  }

  SYMPHONY_DELETE_METHOD(lattice_node(lattice_node const&));
  SYMPHONY_DELETE_METHOD(lattice_node(lattice_node&&));
  SYMPHONY_DELETE_METHOD(lattice_node& operator=(lattice_node const&));
  SYMPHONY_DELETE_METHOD(lattice_node& operator=(lattice_node&&));

public:

  virtual ~lattice_node();

  bool get_parent_sees_tasks() {return _parent_sees_tasks;};

  parent_list& get_parents() {return _parents;};

  children_list& get_children() {return _children;};

  void set_parent_sees_tasks(bool value) {_parent_sees_tasks = value;};

  group_misc::meet_db* get_meet_db() {return _meet_db;};

  size_t get_order() {return _order;};

  group_signature& get_signature() {return _signature;};

  void set_meet_db(group_misc::meet_db* db) {
    SYMPHONY_INTERNAL_ASSERT(_meet_db == nullptr,
        "Meet database must be null inorder to be created");
    _meet_db = db;
  };

  bool is_for_leaf() const {
    return (_order == 1);
  }

};

class lattice_node_leaf: public lattice_node
{
protected:

  friend class ::symphony::internal::group;

  explicit lattice_node_leaf(size_t id):
    lattice_node(id, 1, &_inlined_meet_db),
    _inlined_meet_db (id) {
  }
private:
  group_misc::meet_db _inlined_meet_db;
};

};
};
};
