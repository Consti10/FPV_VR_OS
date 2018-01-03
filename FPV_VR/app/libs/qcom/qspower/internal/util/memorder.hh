#pragma once

#include <atomic>

#include <qspower/internal/util/macros.hh>

namespace qspower
{
// We need a fast way to disable any memory order optimization to see
// whether a bug could be caused by bad synchronization.
#ifdef QSPOWER_USE_SEQ_CST

    using mem_order                                                    = std::memory_order;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_relaxed = std::memory_order::memory_order_seq_cst;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_consume = std::memory_order::memory_order_seq_cst;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_acquire = std::memory_order::memory_order_seq_cst;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_release = std::memory_order::memory_order_seq_cst;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_acq_rel = std::memory_order::memory_order_seq_cst;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_seq_cst = std::memory_order::memory_order_seq_cst;

#else

    using mem_order                                                    = std::memory_order;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_relaxed = std::memory_order::memory_order_relaxed;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_consume = std::memory_order::memory_order_consume;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_acquire = std::memory_order::memory_order_acquire;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_release = std::memory_order::memory_order_release;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_acq_rel = std::memory_order::memory_order_acq_rel;
    static QSPOWER_CONSTEXPR_CONST std::memory_order mem_order_seq_cst = std::memory_order::memory_order_seq_cst;

#endif

}; // namespace
