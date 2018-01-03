// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/queues/bounded_lfqueue.hh>
#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/memorder.hh>

#define SYMPHONY_LFQ_FORCE_SEQ_CST 0

#if SYMPHONY_LFQ_FORCE_SEQ_CST
#define SYMPHONY_LFQ_MO(x) symphony::mem_order_seq_cst
#else
#define SYMPHONY_LFQ_MO(x) x
#endif

namespace symphony {
namespace internal {
namespace bbuf{

  template<typename T> class bounded_buf;
};
namespace lfq{

 template<typename T> class lfq;

template<typename T>
class lfq_node {
public:

  typedef typename internal::blfq::blfq_size_t<T, sizeof(T) <= sizeof(size_t) > container_type;

  explicit lfq_node(size_t log_size) : _c(log_size), _next(nullptr) {}

  SYMPHONY_DELETE_METHOD(lfq_node(lfq_node const&));
  SYMPHONY_DELETE_METHOD(lfq_node(lfq_node &&));
  SYMPHONY_DELETE_METHOD(lfq_node& operator=(lfq_node const&));
  SYMPHONY_DELETE_METHOD(lfq_node& operator=(lfq_node &&));

  lfq_node<T>* get_next(){ return _next.load(SYMPHONY_LFQ_MO(symphony::mem_order_relaxed)); }

  bool set_next(lfq_node<T>* const& new_val){

    lfq_node<T>* old_val = nullptr;
    if(_next.compare_exchange_strong(old_val, new_val, SYMPHONY_LFQ_MO(symphony::mem_order_relaxed))){
      return true;
    }

    return false;
  }

  size_t get_log_node_size() const{ return _c.get_log_array_size();}

  size_t get_max_node_size() const{ return _c.get_max_array_size();}
private:

  container_type _c;

  std::atomic <lfq_node<T>*> _next;

  friend class lfq<T>;

  friend class symphony::internal::bbuf::bounded_buf<T>;

};

template<typename T>
class lfq{
public:
  typedef lfq_node<T> blfq_node;
  typedef T value_type;

 explicit lfq(size_t log_size = 12):
  _head(nullptr),
  _tail(nullptr),
  _original_head(nullptr){
    _head = new blfq_node(log_size);
    _tail.store(_head);
    _original_head = _head;
  }

  virtual ~lfq(){

    SYMPHONY_INTERNAL_ASSERT(_original_head != nullptr, "Error. Head is nullptr");
    while(_original_head != nullptr){
      auto cur = _original_head;
      _original_head = cur->_next;
      delete cur;
    }
  }

  SYMPHONY_DELETE_METHOD(lfq(lfq const&));
  SYMPHONY_DELETE_METHOD(lfq(lfq &&));
  SYMPHONY_DELETE_METHOD(lfq& operator=(lfq const&));
  SYMPHONY_DELETE_METHOD(lfq& operator=(lfq &&));

  size_t push(value_type const& v){

    size_t full_nodes_encountered = 0;

    while(true){
      auto current_blfq_node = _tail.load(SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));

      auto next_blfq_node =  current_blfq_node->get_next();

      if(next_blfq_node != nullptr){
        _tail.compare_exchange_strong(current_blfq_node, next_blfq_node, SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));
        continue;
      }

      auto sz = current_blfq_node->_c.push(v, true);
      if(sz != 0){
        return (sz + (full_nodes_encountered * current_blfq_node->get_max_node_size()));
      }
      else{

        full_nodes_encountered++;
      }

      auto new_blfq_node = new blfq_node(current_blfq_node->get_log_node_size());

      sz = new_blfq_node->_c.push(v, true);
      SYMPHONY_INTERNAL_ASSERT(sz != 0, "Push into a local lfq_node. Should always succeed");

      if(current_blfq_node->set_next(new_blfq_node)){

        _tail.compare_exchange_strong(current_blfq_node, new_blfq_node, SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));
        return (sz + (full_nodes_encountered * current_blfq_node->get_max_node_size()));
      }
      else{

        delete new_blfq_node;
      }
    }
  }

  size_t pop(value_type& r){
    while(true){
      auto current_blfq_node = _head.load(SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));
      auto sz = current_blfq_node->_c.pop(r, true);
      if(sz != 0){
        return sz;
      }

      auto next_blfq_node = current_blfq_node->get_next();
      if(next_blfq_node == nullptr){

        return sz;
      }

      sz = current_blfq_node->_c.pop(r, true);
      if(sz != 0){
        return sz;
      }

      _head.compare_exchange_strong(current_blfq_node, next_blfq_node, SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));
    }
  }

  size_t head_node_size() const{
    auto current_blfq_node = _head.load(SYMPHONY_LFQ_MO(symphony::mem_order_relaxed));
    return current_blfq_node->_c.size();
  }

private:

  std::atomic<blfq_node*> _head;

  std::atomic<blfq_node*> _tail;

  blfq_node* _original_head;
};

};
};
};
