// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <array>
#include <atomic>

#include <symphony/internal/util/debug.hh>
#include <symphony/internal/util/memorder.hh>
#include <symphony/internal/util/popcount.h>

namespace symphony {
namespace internal {
namespace testing {
class concurrent_dense_bitmap_tester;
};

class concurrent_dense_bitmap {

  struct chunk {

    typedef uint32_t element_type;
    typedef std::atomic<element_type> atomic_element_type;

    constexpr static size_t s_num_elements = 4;
    constexpr static size_t s_bits_per_element = (sizeof(element_type) << 3);
    constexpr static size_t s_num_bits = s_bits_per_element * s_num_elements;

    std::atomic<chunk*> _next;
    std::array<atomic_element_type, s_num_elements> _bitmaps;

    chunk():
      _next(nullptr),
      _bitmaps() {

      for(auto& elem : _bitmaps)
        elem = 0;
    }

    bool set_first(size_t& pos);

    inline void reset(size_t pos);

    inline bool test(size_t pos) const;

    size_t popcount() const;

    SYMPHONY_DELETE_METHOD(chunk& operator=(chunk& chnk));
    SYMPHONY_DELETE_METHOD(chunk(chunk const&));
    ~chunk(){}
  };

  chunk _chunk;

  inline chunk* find_chunk(size_t num_chunk);
  inline const chunk* find_chunk_ro(size_t num_chunk) const;

  SYMPHONY_DELETE_METHOD(concurrent_dense_bitmap&
                     operator=(concurrent_dense_bitmap& other));
  SYMPHONY_DELETE_METHOD(concurrent_dense_bitmap
                     (concurrent_dense_bitmap const& other));

public:

  typedef testing::concurrent_dense_bitmap_tester tester;

  concurrent_dense_bitmap()
    :_chunk() {}

  size_t set_first();

  void reset(size_t index);

  bool test(size_t index) const;

  size_t popcount();

  ~concurrent_dense_bitmap();

  friend class testing::concurrent_dense_bitmap_tester;
};

inline concurrent_dense_bitmap::chunk*
concurrent_dense_bitmap::find_chunk(size_t chunk_index)
{
  chunk* curr_chunk = &_chunk;

  for(size_t i = 0; i < chunk_index; ++i) {
    curr_chunk = curr_chunk->_next.load(symphony::mem_order_acquire);
  }

  return curr_chunk;
}

inline const concurrent_dense_bitmap::chunk*
concurrent_dense_bitmap::find_chunk_ro(size_t chunk_index) const
{
  const chunk* curr_chunk = &_chunk;

  for(size_t i = 0; i < chunk_index; ++i) {
    curr_chunk = curr_chunk->_next.load(symphony::mem_order_acquire);
  }

  return curr_chunk;
}

inline void concurrent_dense_bitmap::chunk::reset(size_t pos)
{
  size_t num_elem = pos / s_bits_per_element;
  size_t bit_index = pos % s_bits_per_element;
  _bitmaps[num_elem].fetch_and(~(1 << bit_index), symphony::mem_order_acq_rel);
}

inline bool
concurrent_dense_bitmap::chunk::test(size_t pos) const
{
  size_t num_element = pos / s_bits_per_element;
  size_t bit_index = pos % s_bits_per_element;

  size_t element = _bitmaps[num_element].load(symphony::mem_order_acquire);
  size_t mask =  1 << bit_index;

  return static_cast<size_t>( (element & mask) > 0);
}

};
};
