// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/storage/storagemap.hh>

namespace symphony {
namespace internal {

int task_key_create(storage_key*, void (*dtor)(void*));
int task_set_specific(storage_key, void const*);
void* task_get_specific(storage_key);

};
};
