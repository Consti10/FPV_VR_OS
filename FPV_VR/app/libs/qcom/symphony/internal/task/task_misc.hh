// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <vector>

#include <symphony/internal/legacy/types.hh>
#include <symphony/internal/task/functiontraits.hh>
#include <symphony/internal/task/task_shared_ptr.hh>
#include <symphony/internal/task/task_state.hh>
#include <symphony/internal/util/debug.hh>

namespace symphony {
namespace internal {

class task;
class scheduler;
class task_bundle_dispatch;

namespace testing {

class SuccessorListSuite;
class tasks_tests;

};

namespace task_internal {

class lock_guard {

  task& _task;
  const state_snapshot _snapshot;

  inline explicit lock_guard(task& task);

  inline lock_guard(task& task, state_snapshot snapshot);

  friend class ::symphony::internal::task;

public:

  state_snapshot get_snapshot() const { return _snapshot; };

  inline ~lock_guard();
};

struct release_manager {

  static void release(task* t);

private:

  static bool debug_is_ok_to_release(task* t);
};

class finish_after_state {
public:
  task_shared_ptr _finish_after_stub_task;

  std::vector<group*> _finish_after_groups;

  finish_after_state() :
    _finish_after_stub_task(nullptr),
    _finish_after_groups(0) {}

  void cleanup_groups();

  ~finish_after_state() {
    cleanup_groups();
  }
};

class successor_list {

  class dependency {
    public:
      dependency():
        _succ(nullptr),
        _move(false),
        _dst(nullptr) {}

      explicit dependency(task* succ):
        _succ(succ),
        _move(false),
        _dst(nullptr) {}

      ~dependency() {}

      dependency(task* succ, void* dst, bool move):
        _succ(succ),
        _move(move),
        _dst(dst) {}

      dependency(const dependency& other)
        : _succ(other._succ),
        _move(other._move),
        _dst(other._dst) {}

      dependency& operator=(const dependency& other) {
        _succ = other._succ;
        _move = other._move;
        _dst = other._dst;
        return *this;
      }

      dependency(dependency&& other)
        : _succ(other._succ),
        _move(other._move),
        _dst(other._dst) {}

      dependency& operator=(dependency&& other) {
        _succ = other._succ;
        _move = other._move;
        _dst = other._dst;
        return *this;
      }

      void update(task* t) {
        _succ = t;
      }

      void update(task* t, void* dst, bool move) {
        _succ = t;
        _dst = dst;
        _move = move;
      }

      template <typename ReturnType>
      void notify(void* val) {
        if (_dst != nullptr) {
          propagate_data<ReturnType>(val);
        }
      }

      task*& get_succ() {
        return _succ;
      }

    private:
      task* _succ;
      bool _move;
      void* _dst;

      template <typename ReturnType>
      void propagate_data(void* val, typename std::enable_if<!std::is_void<ReturnType>::value, ReturnType>::type* = nullptr) {
        if (_move) {
          *(static_cast<ReturnType*>(_dst)) = std::move(*(static_cast<ReturnType*>(val)));
        } else {
          *(static_cast<ReturnType*>(_dst)) = *(static_cast<ReturnType*>(val));
        }
      }

      template <typename ReturnType>
      void propagate_data(void*, typename std::enable_if<std::is_void<ReturnType>::value, ReturnType>::type* = nullptr) {

      }

  };

  class bucket {

  public:

    using size_type =  std::uint32_t;

    static SYMPHONY_CONSTEXPR_CONST size_type s_max_entries = 8;

    bucket():
      _dependencies(),
      _next(nullptr),
      _offset(0) {
    }

    bucket(task* t, void* dst, bool move, bucket* next) :
      _dependencies(),
      _next(next),
      _offset(1) {

      SYMPHONY_INTERNAL_ASSERT(next != nullptr, "Invalid null pointer.");

      _dependencies[0].update(t, dst, move);
    }

    ~bucket() {}

    size_type get_offset() const { return _offset; }

    bool can_add_dependency() const { return _offset < s_max_entries; }

    void insert(task* t, void* dst, bool move = false) {
      SYMPHONY_INTERNAL_ASSERT(can_add_dependency() == true, "No entries available in bucket.");
      SYMPHONY_INTERNAL_ASSERT(_dependencies[_offset].get_succ() == nullptr, "Bucket already taken.");
      _dependencies[_offset].update(t, dst, move);
      _offset++;
    }

    bucket* get_next() const { return _next; }

    dependency& operator[](size_type index)  {
      SYMPHONY_INTERNAL_ASSERT(index < s_max_entries, "Out of bounds: %d", static_cast<int>(index));
      return _dependencies[index];
    }

  private:

    SYMPHONY_DELETE_METHOD(bucket(bucket const&));
    SYMPHONY_DELETE_METHOD(bucket(bucket&&));
    SYMPHONY_DELETE_METHOD(bucket& operator=(bucket const&));
    SYMPHONY_DELETE_METHOD(bucket& operator=(bucket&&));

    dependency _dependencies[s_max_entries];
    bucket* _next;
    size_type _offset;
  };

  class state {

    using raw_type = uintptr_t;

    static SYMPHONY_CONSTEXPR_CONST raw_type s_bucket_mask = 1;
    static SYMPHONY_CONSTEXPR_CONST raw_type s_empty_mask = 2;

    raw_type _state;

  public:

    bool is_empty() {
      return _state == s_empty_mask;
    }

    void set_empty() {
      _state = s_empty_mask;
    }

    void set_first_bucket(bucket *b) {
      SYMPHONY_INTERNAL_ASSERT((reinterpret_cast<uintptr_t>(b) & s_bucket_mask) == 0,
                           "Unexpected bucket pointer address: %p",
                           b);

      _state = reinterpret_cast<uintptr_t>(b) | s_bucket_mask;
    }

    bool has_buckets() const {
      return (_state & s_bucket_mask) != 0;
    }

    bucket* get_first_bucket() {
      SYMPHONY_INTERNAL_ASSERT(has_buckets(), "Can't return a bucket becasue there isn't any.");
      return reinterpret_cast<bucket*>(_state & ~s_bucket_mask);
    }

    static task* create_initial_state() {
      return reinterpret_cast<task*>(s_empty_mask);
    }

    SYMPHONY_DELETE_METHOD(state());
    SYMPHONY_DELETE_METHOD(state(state const&));
    SYMPHONY_DELETE_METHOD(state(state&&));
    SYMPHONY_DELETE_METHOD(state& operator=(state const&));
    SYMPHONY_DELETE_METHOD(state& operator=(state&&));

  };

SYMPHONY_GCC_IGNORE_BEGIN("-Wstrict-aliasing")

  state* get_state() {
    return reinterpret_cast<state*>(&_inlined_tasks[0]);
  }

  state* get_state() const {
    return reinterpret_cast<state*>(&(const_cast<successor_list*>(this)->_inlined_tasks[0]));
  }
SYMPHONY_GCC_IGNORE_END("-Wstrict-aliasing")

  static_assert(sizeof(task*) == sizeof(state), "task* should be as big as state");
  static_assert(sizeof(task*) == sizeof(uintptr_t), "task* should be as big as uinptr_t");

public:

  using size_type = size_t;

private:

  static SYMPHONY_CONSTEXPR_CONST size_type s_bucket_size = bucket::s_max_entries;
  static SYMPHONY_CONSTEXPR_CONST size_type s_max_inlined_tasks = 2;

  static_assert(s_max_inlined_tasks >= 1, "Not enough inlined tasks.");
  static_assert(s_bucket_size >= s_max_inlined_tasks + 1,
                "Need to be able to at least store the inlined tasks plus one.");

public:

#ifndef _MSC_VER

  successor_list() :
    _inlined_tasks { state::create_initial_state(), nullptr } {
  }

#else

  successor_list() :
    _inlined_tasks() {
    _inlined_tasks[0] = state::create_initial_state();
  }

#endif

  ~successor_list() {
    SYMPHONY_INTERNAL_ASSERT(is_empty(), "Reset list before deleting it.");
    SYMPHONY_INTERNAL_ASSERT(!has_buckets(), "Overflow buckets exist.");
  }

  bool is_empty() {
    return get_state()->is_empty();
  }

  successor_list(successor_list&& other) {
    move_elements(other);
  }

  successor_list& operator=(successor_list&& other) {
    move_elements(other);
    return *this;
  }

  void add_control_dependency(task* t);

  void add_data_dependency(task* t, void* dst, bool move);

  template <typename ReturnType>
  void propagate_return_value(void* val) {
    if (is_empty())
      return;

    if (has_buckets() == false) {

      return;
    }

    auto b = get_first_bucket();
    do {
      for (bucket::size_type i = 0; i < bucket::s_max_entries; i++) {
        if ((*b)[i].get_succ() == nullptr)
          continue;
        (*b)[i].notify<ReturnType>(val);
      }

      b  = b->get_next();
    } while (b != nullptr);
  }

  inline void propagate_exception(task* t);

  inline void predecessor_finished(bool canceled, scheduler* sched);

  void gpu_predecessor_finished(scheduler* sched, task_bundle_dispatch* tbd);

  template<typename Container>
  void add_to_container(Container& c) {

    if (is_empty())
      return;

    auto l = [&c] (task* t) {
      c.push(t);
    };

    for_each_successor(std::forward<decltype(l)>(l));
  }

  std::string to_string();

private:

  void reset();

  void add_dependency(task* t, void* dst, bool move = false);

  void add_dependency_in_bucket(task* t, void* dst, bool move = false);

  template<typename F>
  void apply_for_each_and_clear(F&& f) {

    using traits = typename ::symphony::internal::function_traits<F>;

    static_assert(traits::arity::value == 1, "F can only have one argument. ");
    static_assert(std::is_same<typename traits::return_type, void>::value,
                  "F must return void. ");

    SYMPHONY_INTERNAL_ASSERT(!is_empty(), "List shouldn't be empty by now");

    if (has_buckets() == false) {
      for (size_t i = 0; i < s_max_inlined_tasks; ++i) {
        if (_inlined_tasks[i] == nullptr)
          continue;
        f(_inlined_tasks[i]);
        _inlined_tasks[i] = nullptr;
      }

      get_state()->set_empty();
      return;
    }

    auto b = get_first_bucket();
    do {
      for (bucket::size_type i = 0; i < bucket::s_max_entries; ++i) {

        if ((*b)[i].get_succ() == nullptr)
          continue;

        f((*b)[i].get_succ());
      }

      auto next = b->get_next();
      delete b;
      b = next;

    } while (b != nullptr);

    get_state()->set_empty();
  }

  template<typename F>
  void for_each_entry(F&& f) {

    using traits = ::symphony::internal::function_traits<F>;

    static_assert(traits::arity::value == 1, "F can only have one argument. ");
    static_assert(std::is_same<typename traits::return_type, void>::value,
                  "F must return void. ");

    if (is_empty())
      return;

    if (has_buckets() == false) {
      for (size_t i = 0; i < s_max_inlined_tasks; ++i) {
        f(&_inlined_tasks[i]);
      }
      return;
    }

    auto b = get_first_bucket();
    do {
      for (bucket::size_type i = 0; i < bucket::s_max_entries; ++i)
        f(&(*b)[i].get_succ());

      b  = b->get_next();
    } while (b != nullptr);
  }

  template<typename F>
  void for_each_successor(F&& f) {

    using traits = ::symphony::internal::function_traits<F>;

    static_assert(traits::arity::value == 1, "F can only have one argument. ");
    static_assert(std::is_same<typename traits::return_type, void>::value,
                  "F must return void. ");

    auto l = [&f](task** t) {
      if (*t != nullptr)
        f(*t);
    };

    for_each_entry(l);
  }

  void set_first_bucket(bucket* b) {
    get_state()->set_first_bucket(b);
  }

  bool has_buckets() const {
    return get_state()->has_buckets();
  }

  bucket* get_first_bucket() const {
    return get_state()->get_first_bucket();
  }

  void move_elements(successor_list& other) {
    for (size_type i = 0; i < s_max_inlined_tasks; ++i) {
      _inlined_tasks[i] = other._inlined_tasks[i];
      other._inlined_tasks[i] = nullptr;
    }
    other.set_empty();
  }

  void reset_inlined_ptr(size_type pos) {
    _inlined_tasks[pos] = nullptr;
  }

  void set_empty() {
    get_state()->set_empty();
  }

  size_type get_num_buckets() const;

  size_type get_num_successors();

  SYMPHONY_DELETE_METHOD(successor_list(successor_list const& other));
  SYMPHONY_DELETE_METHOD(successor_list& operator=(successor_list const& other));

  task* _inlined_tasks[s_max_inlined_tasks];

  friend class testing::SuccessorListSuite;
  friend class testing::task_tests;

};

class blocking_code_container_base {
public:
  virtual void execute() = 0;
  virtual void cancel() = 0;
  virtual ~blocking_code_container_base() {}
};

template<typename BlockingFunction, typename CancelFunction>
class blocking_code_container : public blocking_code_container_base {
  BlockingFunction _bf;
  CancelFunction   _cf;
public:
  blocking_code_container(BlockingFunction&& bf, CancelFunction&& cf) :
    _bf(std::forward<BlockingFunction>(bf)),
    _cf(std::forward<CancelFunction>(cf))
  {}

  virtual void execute() { _bf(); }
  virtual void cancel() { _cf(); }
  virtual ~blocking_code_container() {}

  SYMPHONY_DELETE_METHOD(blocking_code_container(blocking_code_container const&));
  SYMPHONY_DELETE_METHOD(blocking_code_container& operator=(blocking_code_container const&));
  SYMPHONY_DELETE_METHOD(blocking_code_container(blocking_code_container&&));
  SYMPHONY_DELETE_METHOD(blocking_code_container& operator=(blocking_code_container&&));
};

};
};
};
