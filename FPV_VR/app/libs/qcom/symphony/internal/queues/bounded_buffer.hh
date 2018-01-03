// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/queues/bounded_lfqueue.hh>
#include <symphony/internal/queues/lfqueue.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/memorder.hh>

#define SYMPHONY_BBUF_FORCE_SEQ_CST 0

#if SYMPHONY_BBUF_FORCE_SEQ_CST
#define SYMPHONY_BBUF_MO(x) symphony::mem_order_seq_cst
#else
#define SYMPHONY_BBUF_MO(x) x
#endif

namespace symphony {
namespace internal {

namespace testing{

template<typename T, template<typename > class Q>
class execute_test_adaptor;
};

namespace bbuf{

template<typename T>
class bounded_buf{

public:
  typedef symphony::internal::lfq::lfq_node<T> buf_node;
  typedef T value_type;

private:

  size_t push_with_overwrite_return(value_type const& v, value_type& r){
    while (true) {
      auto current_bbuf_node = _active.load(
        SYMPHONY_BBUF_MO(symphony::mem_order_acquire));
      auto res = current_bbuf_node->_c.produce(v, r);
      if (res > 0) {
        return res;
      }

    }
  }

public:
 explicit bounded_buf(size_t log_size = 12):
  _active(nullptr),
  _passive(nullptr),
  _original_head(nullptr){
    _active = new buf_node(log_size);
    _original_head = _active;
  }

  virtual ~bounded_buf(){

    SYMPHONY_INTERNAL_ASSERT(_original_head != nullptr, "Error. Head is nullptr");
    while(_original_head != nullptr){
      auto cur = _original_head;
      _original_head = cur->_next;
      delete cur;
    }
  }

  SYMPHONY_DELETE_METHOD(bounded_buf(bounded_buf const&));
  SYMPHONY_DELETE_METHOD(bounded_buf(bounded_buf &&));
  SYMPHONY_DELETE_METHOD(bounded_buf& operator=(bounded_buf const&));
  SYMPHONY_DELETE_METHOD(bounded_buf& operator=(bounded_buf &&));

  size_t push(value_type const& v){
    value_type res;
    return(push_with_overwrite_return(v,res));
  }

  size_t pop(value_type& r){
    size_t sz = 0;

    auto current_bbuf_node = _passive.load(SYMPHONY_BBUF_MO(symphony::mem_order_relaxed));
    if(current_bbuf_node != nullptr){

      sz = current_bbuf_node->_c.consume(r);
      if(sz != 0){
         return sz;
      }

      _passive.store(nullptr, SYMPHONY_BBUF_MO(symphony::mem_order_relaxed));
      return 0;
    }

    current_bbuf_node = _active.load(SYMPHONY_BBUF_MO(symphony::mem_order_relaxed));

    SYMPHONY_INTERNAL_ASSERT( current_bbuf_node != nullptr ,"Error. Active node is NULL");

    auto new_bbuf_node = new buf_node(current_bbuf_node->get_log_node_size());

    _active.store(new_bbuf_node, SYMPHONY_BBUF_MO(symphony::mem_order_release));

    current_bbuf_node->_c.close();

    current_bbuf_node->_next.store(new_bbuf_node, SYMPHONY_BBUF_MO(symphony::mem_order_relaxed));

    sz = current_bbuf_node->_c.consume(r);
    if(sz != 0){

      _passive.store(current_bbuf_node, SYMPHONY_BBUF_MO(symphony::mem_order_relaxed));
    }
    return sz;
  }

private:

  std::atomic<buf_node*> _active;

  std::atomic<buf_node*> _passive;

  buf_node* _original_head;

  template<typename T1, template<typename > class Q>
  friend class symphony::internal::testing::execute_test_adaptor;
};

};
};
};
