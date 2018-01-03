// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <atomic>

#include <symphony/internal/util/macros.hh>

namespace symphony {

#ifdef SYMPHONY_USE_SEQ_CST

using mem_order = std::memory_order;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_relaxed = std::memory_order::memory_order_seq_cst;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_consume = std::memory_order::memory_order_seq_cst;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_acquire = std::memory_order::memory_order_seq_cst;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_release = std::memory_order::memory_order_seq_cst;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_acq_rel = std::memory_order::memory_order_seq_cst;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_seq_cst = std::memory_order::memory_order_seq_cst;

#else

using mem_order = std::memory_order;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_relaxed = std::memory_order::memory_order_relaxed;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_consume = std::memory_order::memory_order_consume;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_acquire = std::memory_order::memory_order_acquire;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_release = std::memory_order::memory_order_release;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_acq_rel = std::memory_order::memory_order_acq_rel;
static SYMPHONY_CONSTEXPR_CONST std::memory_order mem_order_seq_cst = std::memory_order::memory_order_seq_cst;

#endif

};
