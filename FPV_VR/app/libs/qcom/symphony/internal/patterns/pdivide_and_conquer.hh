// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/patterns/common.hh>

namespace symphony {

namespace internal {

template <typename Problem, typename Solution>
struct dnc_node {
  explicit dnc_node(Problem p) : _problem(p), _solution(), _subproblems() {}

  dnc_node(const dnc_node& other) {
    _problem = other._problem;
    _solution = other._solution;
    _subproblems = other._subproblems;
  }

  dnc_node& operator=(const dnc_node& other) {
    if (this != &other) {
      _problem = other._problem;
      _solution = other._solution;
      _subproblems = other._subproblems;
    }
    return *this;
  }

  Problem _problem;
  Solution _solution;

  std::vector<std::shared_ptr<dnc_node>> _subproblems;
};

template <typename Problem, typename Solution, typename Fn1,
         typename Fn2, typename Fn3, typename Fn4>
void pdivide_and_conquer_internal(group_ptr g,
                                  task_ptr<> c,
                                  dnc_node<Problem, Solution>* n,
                                  Fn1&& is_base_case,
                                  Fn2&& base, Fn3&& split,
                                  Fn4&& merge,
                                  bool is_base_n) {
  SYMPHONY_INTERNAL_ASSERT(n->_subproblems.empty(),
      "work node should have no subproblems just yet");
  if (is_base_n) {
    SYMPHONY_INTERNAL_ASSERT(c == nullptr, "if base, continuation should be null");
    n->_solution = base(n->_problem);
  } else {
    for (auto i : split(n->_problem)) {

      auto node_i = std::make_shared<dnc_node<Problem, Solution>>(i);
      n->_subproblems.push_back(node_i);

      auto is_base_i = is_base_case(i);
      auto cont_child = is_base_i ? nullptr : symphony::create_task([=]() {

        SYMPHONY_INTERNAL_ASSERT(!node_i->_subproblems.empty(),
            "need atleast one subproblem to merge");
        std::vector<Solution> sols(node_i->_subproblems.size());
        std::transform(node_i->_subproblems.begin(),
            node_i->_subproblems.end(),
            sols.begin(),
            [](std::shared_ptr<dnc_node<Problem, Solution>> m){
              return m->_solution;
            });
        node_i->_solution = merge(i, sols);
      });

      auto task_child = symphony::create_task([=] {

        internal::pdivide_and_conquer_internal(g, cont_child, node_i.get(),
                            is_base_case, base, split, merge, is_base_i);
      });
      internal::c_ptr(task_child)->launch(internal::c_ptr(g), nullptr);

      auto c_pred = is_base_i ? task_child : cont_child;

      c_pred->then(c);
    }
    internal::c_ptr(c)->launch(internal::c_ptr(g), nullptr);
  }
}

template <typename Problem, typename Fn1, typename Fn2,
         typename Fn3, typename Fn4>
void pdivide_and_conquer_internal(group_ptr g,
                                  task_ptr<> c,
                                  Problem p,
                                  Fn1&& is_base_case,
                                  Fn2&& base,
                                  Fn3&& split,
                                  Fn4&& merge,
                                  bool is_base_p) {
  if (is_base_p) {
    SYMPHONY_INTERNAL_ASSERT(c == nullptr, "if base, continuation should be null");
    base(p);
  } else {
    for (auto i : split(p)) {

      auto is_base_i = is_base_case(i);
      auto cont_child = is_base_i ?
        nullptr : symphony::create_task([=]{merge(i);});

      auto task_child = symphony::create_task([=] {
        internal::pdivide_and_conquer_internal(g, cont_child, i, is_base_case,
          base, split, merge, is_base_i);
      });
      internal::c_ptr(task_child)->launch(internal::c_ptr(g), nullptr);

      auto c_pred = is_base_i ? task_child : cont_child;

      c_pred->then(c);
    }
    internal::c_ptr(c)->launch(internal::c_ptr(g), nullptr);
  }
}

template <typename Problem, typename Fn1, typename Fn2, typename Fn3>
void pdivide_and_conquer_internal(group_ptr g,
                                  Problem p,
                                  Fn1&& is_base_case,
                                  Fn2&& base,
                                  Fn3&& split,
                                  bool is_base_p) {
  SYMPHONY_INTERNAL_ASSERT(g != nullptr, "group must not be null");
  if (is_base_p) {
    base(p);
  } else {
    for (auto i : split(p)) {
      auto is_base_i = is_base_case(i);
      auto f = [=] {
        internal::pdivide_and_conquer_internal(g, i, is_base_case,
            base, split, is_base_i);
      };

      auto load = internal::get_load_on_current_thread();
      if (load == 0) {

        g->launch(f);
      } else {

        f();
      }
    }
  }
}

template <typename Problem, typename Solution, typename Fn1,
         typename Fn2, typename Fn3, typename Fn4>
symphony::task_ptr<Solution> pdivide_and_conquer(group_ptr g, Problem p, Fn1&& is_base,
    Fn2&& base, Fn3&& split, Fn4&& merge) {
  if (is_base(p)) {
    return symphony::create_value_task<Solution>(base(p));
  } else {

    auto n = std::make_shared<internal::dnc_node<Problem, Solution>>(p);
    auto cont = symphony::create_task([n, p, merge] {

      std::vector<Solution> sols(n->_subproblems.size());
      std::transform(n->_subproblems.begin(), n->_subproblems.end(),
          sols.begin(),
          [](std::shared_ptr<internal::dnc_node<Problem, Solution>> m){
            return m->_solution;
          });
      n->_solution = merge(p, sols);
      return n->_solution;
    });

    internal::pdivide_and_conquer_internal(g, cont, n.get(), is_base, base,
        split, merge, false );
    return cont;
  }
}

template <typename Problem, typename Fn1, typename Fn2,
         typename Fn3, typename Fn4>
void pdivide_and_conquer(group_ptr g, Problem p, Fn1&& is_base,
    Fn2&& base, Fn3&& split, Fn4&& merge, bool async = false) {
  SYMPHONY_INTERNAL_ASSERT(g == nullptr || async == true,
      "g should be valid only for async invocations");
  if (is_base(p)) {
    base(p);
  } else {
    auto c = symphony::create_task([merge, p] {merge(p);});
    internal::pdivide_and_conquer_internal(g, c, p, is_base, base,
        split, merge, false );
    if (async)
      g->finish_after();
    else
      c->wait_for();
  }
}

template <typename Problem, typename Fn1, typename Fn2, typename Fn3>
void pdivide_and_conquer(group_ptr g,
                         Problem p,
                         Fn1&& is_base,
                         Fn2&& base,
                         Fn3&& split,
                         bool async = false) {
  SYMPHONY_INTERNAL_ASSERT(g == nullptr || async == true,
      "g should be valid only for async invocations");
  if (is_base(p)) {
    base(p);
  } else {
    if (async || g == nullptr)
      g = symphony::create_group();
    internal::pdivide_and_conquer_internal(g, p, is_base, base,
        split, false );
    if (async)
      g->finish_after();
    else
      g->wait_for();
  }
}

template <typename Problem, typename Solution, typename Fn1,
         typename Fn2, typename Fn3, typename Fn4>
symphony::task_ptr<Solution> pdivide_and_conquer_async(Fn1&& is_base,
    Fn2&& base, Fn3&& split, Fn4&& merge, Problem p) {
  auto g = create_group();
  auto t = symphony::create_task([g, p, is_base, base, split, merge]{
      return symphony::internal::pdivide_and_conquer<Problem, Solution>(g, p, is_base, base, split, merge);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <typename Problem, typename Fn1, typename Fn2,
         typename Fn3, typename Fn4>
symphony::task_ptr<> pdivide_and_conquer_async(Fn1&& is_base,
    Fn2&& base, Fn3&& split, Fn4&& merge, Problem p) {
  auto g = symphony::create_group();
  auto t = symphony::create_task([g, p, is_base, base, split, merge]{
      symphony::internal::pdivide_and_conquer(g, p, is_base, base, split, merge, true);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

template <typename Problem, typename Fn1, typename Fn2, typename Fn3>
symphony::task_ptr<>
pdivide_and_conquer_async(Fn1&& is_base, Fn2&& base, Fn3&& split, Problem p) {

  auto g = symphony::create_group();
  auto t = symphony::create_task([g, p, is_base, base, split]{
      symphony::internal::pdivide_and_conquer(g, p, is_base,
        base, split, true);
  });
  auto gptr = internal::c_ptr(g);
  gptr->set_representative_task(internal::c_ptr(t));
  return t;
}

};
};
