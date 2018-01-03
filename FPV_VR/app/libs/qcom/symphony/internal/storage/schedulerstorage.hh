// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <symphony/internal/storage/storagemap.hh>

namespace symphony {
namespace internal {

typedef unsigned short device_thread_id;

typedef unsigned short hexagon_device_thread_id;

int sls_key_create(storage_key*, void (*dtor)(void*));
int sls_set_specific(storage_key, void const*);
void* sls_get_specific(storage_key);

};
};
