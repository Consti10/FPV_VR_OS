// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <string>

namespace symphony {
namespace internal {

class group;
class task;

namespace unlaunched_task_cache {

size_t get_size();

bool insert(task* t, group* g);

void remove(task* t);

void cancel_tasks_from_group(group* g);

std::string to_string();

void init_utcache();

void shutdown_utcache();

};
};
};
