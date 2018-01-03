// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/legacy/task.hh>

namespace symphony {
namespace internal {

template <typename StubTaskFn>
void task::finish_after_impl(task* pred, task* succ, StubTaskFn&& fn) {
  finish_after_state_ptr& tfa = succ->get_finish_after_state();
  if (!succ->should_finish_after(tfa)) {

    auto c = create_stub_task(fn, pred);
    succ->set_finish_after_state(tfa, c);
  } else {
    auto fa = tfa.get();
    SYMPHONY_INTERNAL_ASSERT(fa->_finish_after_stub_task != nullptr,
        "must have already invoked set_finish_after_state");
    task::add_task_dependence(pred, c_ptr(fa->_finish_after_stub_task));
  }
}

};
};
