// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/compat/compiler_check.h>

#include <symphony/affinity.hh>
#include <symphony/buffer.hh>
#include <symphony/index.hh>
#include <symphony/range.hh>
#include <symphony/runtime.hh>

#include <symphony/group.hh>
#include <symphony/groupptr.hh>
#include <symphony/patterns.hh>
#include <symphony/pointkernel.hh>
#include <symphony/power.hh>

#include <symphony/schedulerstorage.hh>
#include <symphony/scopedstorage.hh>
#include <symphony/taskstorage.hh>
#include <symphony/threadstorage.hh>

#include <symphony/taskfactory.hh>
#include <symphony/texture.hh>
#include <symphony/internal/compat/compat.h>

#include <symphony/internal/task/task.cc.hh>

#ifdef __cplusplus
extern "C" {
#endif
const char *symphony_version();
const char *symphony_scm_revision();
int         symphony_scm_changed();
const char *symphony_scm_version();
const char *symphony_scm_description();

#ifdef __cplusplus
}
#endif
