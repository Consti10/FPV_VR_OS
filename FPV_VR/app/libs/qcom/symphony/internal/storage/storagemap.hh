// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <vector>
#include <memory>

#include <symphony/internal/util/macros.hh>

namespace symphony {
namespace internal {

struct storage_key {
  size_t id;
  void (*dtor)(void*);
};

class storage_map {
private:

  struct task_storage_item {
    task_storage_item(void const* data_, void (*dtor_)(void*))
      : data(data_), dtor(dtor_) {}
    task_storage_item()
      : data(nullptr), dtor(nullptr) {}

    void const* data;
    void (*dtor)(void*);
  };

  std::unique_ptr<std::vector<task_storage_item> > _map;

public:

  storage_map()
    : _map(nullptr) {}

  SYMPHONY_DELETE_METHOD(storage_map(storage_map const&));
  SYMPHONY_DELETE_METHOD(const storage_map& operator=(storage_map const&));

#ifndef _MSC_VER

  SYMPHONY_DELETE_METHOD(storage_map& operator=(storage_map const&) volatile);

#endif

  ~storage_map() {
    dispose();
  }

  void* get_specific(storage_key key) {
    if (_map && key.id < _map->size())
      return const_cast<void*>((*_map)[key.id].data);
    return nullptr;
  }

  void set_specific(storage_key key, void const* value) {
    ensure_map_size(key.id + 1);
    (*_map)[key.id] = task_storage_item(value,key.dtor);
  }

  void dispose() {
    if (!_map)
      return;

    for (auto& item : *_map) {
      if (item.data && item.dtor) {
        (*item.dtor)(const_cast<void*>(item.data));
      }
    }
    _map.reset();
  }

private:

  void ensure_map_size(size_t size) {
    if (!_map) {
      _map = std::unique_ptr<std::vector<task_storage_item>>
        (new std::vector<task_storage_item>(size));
    } else if (_map->size() < size) {
      _map->resize(size, task_storage_item());
    }
  }
};

};
};
