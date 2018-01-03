// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>

#include <symphony/internal/task/group_shared_ptr.hh>
#include <symphony/internal/task/task_shared_ptr.hh>

#include <symphony/internal/legacy/types.hh>

namespace symphony {
namespace internal {
namespace legacy {

group_shared_ptr create_group(std::string const& name);
group_shared_ptr create_group(const char* name);
group_shared_ptr create_group();
group_shared_ptr intersect(group_shared_ptr const& a, group_shared_ptr const& b);
void join_group(group_shared_ptr const& group, task_shared_ptr const& task);
void wait_for(group_shared_ptr const& group);
void spin_wait_for(group_shared_ptr const& group);
void finish_after(group_shared_ptr const& g);
void cancel(group_shared_ptr const& group);
bool canceled(group_shared_ptr const& group);
group_shared_ptr operator&(group_shared_ptr const& a, group_shared_ptr const& b);

};
};
};
