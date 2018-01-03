// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <mutex>
#include <set>

#include <symphony/internal/task/group_signature.hh>
#include <symphony/internal/task/group_shared_ptr.hh>

namespace symphony {
namespace internal {

class group;

namespace group_misc {

enum lattice_lock_policy {
  acquire_lattice_lock,
  do_not_acquire_lattice_lock
};

class lattice {
public:

  typedef std::recursive_mutex lock_type;

private:

  typedef std::set<group*> group_set;
  static lock_type s_mutex;

public:

  static group_shared_ptr create_meet_node(group* a, group* b, group* current_group = nullptr);

  static lock_type& get_mutex() { return s_mutex;}

private:

  static void find_sup(group_signature& new_b, group* ancestor1,
                       group* ancestor2, group_set& sup)
  {
    find_sup(new_b, ancestor1, sup);
    find_sup(new_b, ancestor2, sup);
  };

  static void find_inf(group_signature& new_b, group* ancestor1,
                         group* ancestor2, group_set& inf)
  {
    find_inf(new_b, ancestor1, inf);
    find_inf(new_b, ancestor2, inf);
  }

  static void find_sup(group_signature& new_b, group* ancestor,
                       group_set& sup);

  static void find_sup_rec(group_signature& new_b, size_t order,
                           group* ancestor, group_set& sup, bool& found);

  static void find_inf(group_signature& new_b, group* ancestor,
                       group_set& inf);

  static void find_inf_rec(group_signature& new_b, size_t order, group* node,
                           group_set& inf);

  static void insert_in_lattice(group* new_group, group_set const& sup,
                                group_set const& inf);

  static void insert_in_lattice_childfree_leaves(group* new_group,
                                                 group* a, group* b);

  static void insert_in_lattice_childfree_parents(group* new_group,
                                                  group_set const& sup);

  static void insert_in_lattice_leaves(group* new_group, group* a,
                                              group* b, group_set const& inf);

};

};
};
};
