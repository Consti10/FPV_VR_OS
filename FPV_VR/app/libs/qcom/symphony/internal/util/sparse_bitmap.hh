// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <initializer_list>
#include <string>

#include <symphony/internal/util/macros.hh>
#include <symphony/internal/util/popcount.h>

namespace symphony {
namespace internal {

class sparse_bitmap {

public:

  typedef size_t hash_t;

private:
  class chunk {

  public:

    typedef size_t bitmap_t;

    const static size_t s_num_bits = (sizeof(bitmap_t) << 3);

    bitmap_t _bitmap;
    size_t _pos;
    chunk* _next;

    chunk(bitmap_t bmp, size_t pos, chunk* n = nullptr):
      _bitmap(bmp),
      _pos(pos),
      _next(n) {}

    chunk():
      _bitmap(0),
      _pos(0),
      _next(nullptr) {}

    void initialize(bitmap_t bmp, size_t pos, chunk* n = nullptr) {
      _bitmap = bmp;
      _pos = pos;
      _next = n;
    }

    SYMPHONY_DELETE_METHOD(chunk& operator=(chunk& chnk));
    SYMPHONY_DELETE_METHOD(chunk(chunk& chnk));
    ~chunk(){}

  };

private:

  chunk _chunks;

  size_t _hash_value;

  void set_bitmap(chunk::bitmap_t value) {
    clear();
    _chunks.initialize(value, 0, nullptr);

    _hash_value = static_cast<hash_t>(value);
  }

public:

  typedef chunk::bitmap_t bitmap_t;

  sparse_bitmap():
    _chunks(),
    _hash_value(0) {}

  struct singleton_t {};
  static singleton_t const singleton;

  explicit sparse_bitmap(size_t value):
    _chunks(value, 0, nullptr),
    _hash_value(static_cast<hash_t>(value)) {
  }

  sparse_bitmap(size_t bit, singleton_t const&):
    _chunks(),
    _hash_value(0) {
    first_set(bit);
  }

  explicit sparse_bitmap(std::initializer_list<size_t> list):
    _chunks(),
    _hash_value(0){
    for (auto& index: list)
      set(index);
  }

  sparse_bitmap(sparse_bitmap&& other):
  _chunks(other._chunks._bitmap,
          other._chunks._pos,
          std::move(other._chunks._next)),
  _hash_value(other._hash_value) {

    other._chunks.initialize(0, 0, nullptr);
    other._hash_value = 0;
  }

  sparse_bitmap(const sparse_bitmap& bm):
    _chunks(bm._chunks._bitmap, bm._chunks._pos, nullptr),
    _hash_value(bm._hash_value) {

    chunk* copy_chunk = bm._chunks._next;
    chunk* new_chunk = nullptr;
    chunk* prev_chunk = nullptr;
    while (copy_chunk != nullptr) {
      new_chunk = new chunk(copy_chunk->_bitmap,
          copy_chunk->_pos, nullptr);
      if (_chunks._next == nullptr)
        _chunks._next = new_chunk;
      if (prev_chunk != nullptr) {
        prev_chunk->_next = new_chunk;
      }
      prev_chunk = new_chunk;
      copy_chunk = copy_chunk->_next;
    }
  }

  ~sparse_bitmap(){
    clear();
  }

  SYMPHONY_DELETE_METHOD(sparse_bitmap& operator=(const sparse_bitmap& bm));

  sparse_bitmap& operator=(sparse_bitmap&& other) {
    clear();

    _chunks.initialize(other._chunks._bitmap,
        other._chunks._pos,
        std::move(other._chunks._next));
    _hash_value = other._hash_value;

    other._chunks.initialize(0, 0, nullptr);
    return *this;
  }

  sparse_bitmap operator|(const sparse_bitmap& bm) {
    sparse_bitmap result;
    result.set_union(*this, bm);
    return result;
  }

  bool set(size_t index);

  bool first_set(size_t index) {
    if (index < sparse_bitmap::chunk::s_num_bits) {

      _chunks._bitmap = one_bit_bitmap(index);
    }
    else {

      set(index);
    }
    return true;
  }

  hash_t get_hash_value() const {return _hash_value;};

  void clear() {

    chunk* current_chunk = _chunks._next;
    chunk* next_chunk = current_chunk;
    while(current_chunk) {
      current_chunk = current_chunk->_next;
      delete next_chunk;
      next_chunk = current_chunk;
    }
    _chunks._bitmap = 0;
    _chunks._pos = 0;
    _chunks._next = nullptr;
    _hash_value = 0;
  }

  size_t popcount() const;

  bool is_singleton() const {

    return (_chunks._next == nullptr &&
        (symphony_popcountw(_chunks._bitmap) == 1));
  };

  bool is_empty() const {
    return (_chunks._bitmap == 0);
  }

  inline bitmap_t one_bit_bitmap(size_t index) {
    return (static_cast<bitmap_t>(1) << index);
  };

  chunk* insert_chunk(chunk* prev, chunk* next, bitmap_t bmp, size_t new_pos);

  void recompute_hash();

  bool operator==(const sparse_bitmap& bm) const;

  void set_union(const sparse_bitmap& bm1, const sparse_bitmap& bm2);

  void fast_set_union(const sparse_bitmap& bm1, const sparse_bitmap& bm2) {
    if (bm1._chunks._pos == 0 && bm1._chunks._next == nullptr &&
        bm2._chunks._pos == 0 && bm2._chunks._next == nullptr) {

      _chunks._pos = 0;
      _chunks._next = nullptr;
      _chunks._bitmap = bm1._chunks._bitmap | bm2._chunks._bitmap;
      _hash_value = static_cast<hash_t>(_chunks._bitmap);
    }
    else {

      set_union(bm1, bm2);
    }
  }

  bool subset(sparse_bitmap const& bm) const;

  std::string to_string() const;
};
};
};
