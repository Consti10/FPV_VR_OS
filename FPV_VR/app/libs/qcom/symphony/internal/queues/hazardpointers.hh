// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/tlsptr.hh>

namespace symphony {

namespace internal {

namespace hp {

template<typename T>
struct ListNode {
public:
  ListNode(ListNode* n, T e):
    _next(n),
    _element(e) {
  }
  void next(ListNode* const& n) {
    _next = n;
  }
  ListNode* const& next() const {
    return _next;
  }
  const T& element() const {
    return _element;
  }

  private:
  ListNode* _next;
  T _element;
};

SYMPHONY_GCC_IGNORE_BEGIN("-Weffc++");
template<typename TN, int K>
struct Record {
public:

  Record():
    _owner(0),
    _next(nullptr),
    _active(false),
    _rList(nullptr),
    _rCount(0),
    _nrThreads(0) {
    for(int idx=0; idx != K; idx++)
      _HP[idx] = nullptr;
#ifdef SYMPHONY_DEBUG
    for(int idx=K; idx--;) {
      if(_HP[idx]) {
        SYMPHONY_FATAL("Record not clean");
      }
    }
#endif
  }

  ~Record() {
    ListNode<TN*>* nextNode;
    for(ListNode<TN*>* rList = _rList; rList != nullptr; rList = nextNode) {
      nextNode = rList->next();
      delete rList->element();
      delete rList;
    }
  }

  void push(TN* element) {
    ListNode<TN*>* newNode = new ListNode<TN*>(_rList,element);
    _rList = newNode;
    ++_rCount;
  }

  void pop(TN*& element) {
    ListNode<TN*>* oldHead = _rList;
    _rList = _rList->next();
    element = oldHead->element();
    delete oldHead;
    --_rCount;
  }

  void scan(Record* headHPRecord) {

    SYMPHONY_INTERNAL_ASSERT(headHPRecord->nrThreads()>0, "nrThreads should be > 0");
#ifdef _MSC_VER

    std::vector<TN*> pList_temp (headHPRecord->nrThreads()*K);
    TN** pList = pList_temp.data();
#else

    TN* pList[headHPRecord->nrThreads()*K];
#endif

    TN** pIdx = pList;
    for(Record* hprec = headHPRecord; hprec != nullptr; hprec = hprec->_next) {
      for(int i=0; i<K; ++i) {
        TN* hptr = hprec->_HP[i];
        if(hptr) {
          *pIdx++ = hptr;
        }
      }
    }

    ListNode<TN*>* last = nullptr;
    ListNode<TN*>* node = _rList;
    while(node) {

      bool found = false;
      for(TN** p = pList; p<pIdx; p++) {
        if(*p == node->element()) {
          found = true;
          break;
        }
      }

      if(!found) {

        if(last != nullptr) {
          last->next(node->next());
          delete node->element();
          delete node;
          node = last->next();
        } else {
          _rList = node->next();
          delete node->element();
          delete node;
          node = _rList;
        }
        --_rCount;

      } else {
        last = node;
        node = node->next();
      }
    }
  }

  void helpScan(std::atomic<Record*>& headHPRecord, const int& nodeThreshold) {

    for(Record* hprec = headHPRecord; hprec; hprec = hprec->_next) {

      if(!hprec->tryClaim()) continue;

      while(hprec->rCount()>0) {
        TN* node;

        hprec->pop(node);
        push(node);

        Record* head = headHPRecord;

        if(_rCount >= nodeThreshold) {
          scan(head);
        }
      }

      hprec->release();
    }
  }

  void retire() {
    for(int k=K; k--;) {
      _HP[k] = nullptr;
    }
    release();
  }

  bool isActive() {
    return _active;
  }

  bool tryClaim() {
    if(_active) return false;
    bool b = false;
    b = _active.compare_exchange_strong(b, true);
#ifdef SYMPHONY_DEBUG
    if(b) {

      __sync_synchronize();
      if(_owner != 0) {
        SYMPHONY_FATAL("Thread got claim to already claimed record: %" SYMPHONY_FMT_TID, _owner);
      }
      _owner = thread_id();
    }
#endif
    return b;
  }

  void release() {
#ifdef SYMPHONY_DEBUG
    if(_owner != thread_id()) {
      SYMPHONY_FATAL("FATAL: Thread released record that is not his: %" SYMPHONY_FMT_TID "/%" SYMPHONY_FMT_TID
                , _owner, thread_id()
                );
    }
    _owner = 0;

    __sync_synchronize();
#endif
    _active = false;
  }

  const int& rCount() const {
    return _rCount;
  }

  Record* const& next() const {
    return _next;
  }

  void next(Record* const& n) {
    _next = n;
  }

  TN* const & operator[](size_t slot) const {
    return _HP[slot];
  }

  void setSlot(size_t slot, TN* const& hp) {
    _HP[slot] = hp;
  }

  void clearSlot(size_t slot) {
    _HP[slot] = nullptr;
  }

  void print(std::ostream& out) {
    {
      out << "[";
      if(_active) {
        out << "ACT";
      } else {
        out << "OFF";
      }
      out << "|" << this << "]";
    }
#ifdef SYMPHONY_DEBUG
    {
      out << "[";
      if(_owner != 0) {
        out.setf(out.hex);
        out.width(16);
        out << _owner;
      } else {
        out << "      free      ";
      }
      out << "]";
    }
#endif
    {
      out << "[";
      bool first = true;
      for(int i = 0; i<K; ++i) {
        if(first) {
          first = false;
        } else {
          out <<"|";
        }
        out.width(16);
        out << _HP[i];
      }
      out << "]";
    }
    {
      out << " ";
      bool first = true;
      for(ListNode<TN*>* rList = _rList; rList != nullptr; rList = rList->next()) {
        if(first) {
          first = false;
        } else {
          out <<", ";
        }
        out.width(16);
        out << rList;
      }
    }
    out << std::endl;
  }

  void nrThreads(size_t nThreads) {
    _nrThreads = nThreads;
  }
  const size_t& nrThreads() {
    return _nrThreads;
  }

public:
  uintptr_t _owner;

  TN* _HP[K];
  Record* _next;
  std::atomic<bool> _active;
  ListNode<TN*>* _rList;
  int _rCount;
  size_t _nrThreads;
};
SYMPHONY_GCC_IGNORE_END("-Weffc++");

template<typename TN, int K>
class Manager {
public:
  typedef Record<TN,K>* Record_p;
  static int const MINIMAL_RLIST_SIZE;

  class CheckCleanOnDestroy {
  public:
    explicit CheckCleanOnDestroy(Manager<TN,K>& HPManager): _HPManager(HPManager) {

    }
    ~CheckCleanOnDestroy() {
      for(int idx=K; idx--;) {
        if(_HPManager[idx] != nullptr) {
          std::stringstream s;
          _HPManager.print(s);
          SYMPHONY_FATAL("Record not clean - %s",s.str().c_str());
        }
      }
    }
  private:
    Manager<TN,K>& _HPManager;
  };

public:

  explicit Manager(const int& nodeThreshold = MINIMAL_RLIST_SIZE):
    _headHPRecord(nullptr),
    _recordCount(0),
    _nodeThreshold(nodeThreshold),
    _myhprec(nullptr) {
  }

  ~Manager() {
    Record_p next;
    for(Record_p hprec = _headHPRecord; hprec!=nullptr; hprec = next) {
      next = hprec->next();
      delete hprec;
    }
  }

  void threadInit() {
    SYMPHONY_INTERNAL_ASSERT(!_myhprec, "Please call threadInit() only on an uninitialized thread");
    allocRecord(_myhprec);
  }

  void threadInitAuto() {
    if(_myhprec == nullptr) {
      allocRecord(_myhprec);
    }
  }

  void threadFinishAuto() {
    if(_myhprec != nullptr) {
      threadFinish();
    }
  }

  void threadFinish() {
    SYMPHONY_INTERNAL_ASSERT(_myhprec, "Please call threadInit() on each thread using hp::Manager");
    _myhprec->retire();
    _myhprec = nullptr;
  }

  void retireNode(TN* node) {
    retireNode(_myhprec,node);
  }

  TN* const& operator[](size_t slot) {
    SYMPHONY_INTERNAL_ASSERT(_myhprec, "Please call threadInit() on each thread using hp::Manager");
    return (*_myhprec)[slot];
  }

  void setMySlot(size_t slot, TN* const& hp) {
    _myhprec->setSlot(slot,hp);
    __sync_synchronize();
  }

  void clearMySlot(size_t slot) {
    _myhprec->clearSlot(slot);
  }

  void print() {
    print(std::cerr);
  }
  void print(std::ostream& out) {
    for(Record_p hprec = _headHPRecord; hprec != nullptr; hprec = hprec->next()) {
      hprec->print(out);
    }
  }

  void printSafe(std::ostream& out) {
    for(Record_p hprec = _headHPRecord; hprec != nullptr; hprec = hprec->_next) {
      if(hprec->tryClaim()) {
        hprec->print(out);
      } else {
        out << "Could not claim: " << hprec << std::endl;
      }
    }
  }

  bool isUsed(TN* node) {
    Record_p headHPRecord = _headHPRecord;
    SYMPHONY_INTERNAL_ASSERT(headHPRecord->nrThreads()>0, "nrThreads should be > 0");
#ifdef _MSC_VER

    std::vector<TN*> pList_temp (headHPRecord->nrThreads()*K);
    TN** pList = pList_temp.data();
#else

    TN* pList[headHPRecord->nrThreads()*K];
#endif

    TN** pIdx = pList;
    for(Record_p hprec = headHPRecord; hprec != nullptr; hprec = hprec->_next) {
      for(int i=0; i<K; ++i) {
        TN* hptr = hprec->_HP[i];
        if(hptr == node) {
          return true;
        }
      }
    }

    return false;
  }

  bool isUsedExceptByMe(TN* node) {
    Record_p headHPRecord = _headHPRecord;
    SYMPHONY_INTERNAL_ASSERT(
      headHPRecord->nrThreads()>0,
      "nrThreads should be > 0");
#ifdef _MSC_VER

    std::vector<TN*> pList_temp (headHPRecord->nrThreads()*K);
    TN** pList = pList_temp.data();
#else

    TN* pList[headHPRecord->nrThreads()*K];
#endif

    Record_p myRecord = _myhprec;

    TN** pIdx = pList;
    for(Record_p hprec = headHPRecord; hprec != nullptr; hprec = hprec->_next) {
      if(hprec == myRecord) {
        continue;
      }
      for(int i=0; i<K; ++i) {
        TN* hptr = hprec->_HP[i];
        if(hptr == node) {
          return true;
        }
      }
    }

    return false;
  }

private:
  static void cleanupTrampoline(void* manager) {
    static_cast<symphony::internal::hp::Manager<TN,K>*>(manager)->
      threadFinishAuto();
  }

private:

  void allocRecord(tlsptr<Record<TN,K>>& myhprec) {
    Record_p hprec;
    Record_p oldHead;

    SYMPHONY_INTERNAL_ASSERT(!myhprec, "ERROR: myhprec was already initialized");

    for(hprec = _headHPRecord; hprec!=nullptr; hprec = hprec->next()) {
      if(!hprec->tryClaim()) continue;

      myhprec = hprec;
      return;
    }

    _recordCount++;

    if(_recordCount >= MINIMAL_RLIST_SIZE/2) {
      _nodeThreshold = _recordCount*2;
    }
    hprec = new Record<TN,K>();
    if(!hprec->tryClaim()) {
      SYMPHONY_FATAL("Could not claim my own record");

    }
    do {
      oldHead = _headHPRecord;
      hprec->next(oldHead);
      hprec->nrThreads(_recordCount);
    } while(!_headHPRecord.compare_exchange_strong(oldHead,hprec));
    myhprec = hprec;
    return;
  }

  void retireNode(tlsptr<Record<TN,K>>& myhprec, TN* node) {
    SYMPHONY_INTERNAL_ASSERT(myhprec, "Please call threadInit() on each thread using hp::Manager");
    myhprec->push(node);
    Record_p head = _headHPRecord;
    if(myhprec->rCount() >= _nodeThreshold) {

      myhprec->scan(head);

      myhprec->helpScan(_headHPRecord,_nodeThreshold);
    }
  }

private:

  std::atomic<Record_p> _headHPRecord;

  std::atomic<int> _recordCount;

  int _nodeThreshold;

private:
  tlsptr<Record<TN,K>> _myhprec;
};

template<typename TN,int K>
const int Manager<TN,K>::MINIMAL_RLIST_SIZE = 10;

};

};

};
