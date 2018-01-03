#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Returns the id for the platform's thread
uintptr_t qspower_internal_get_platform_thread_id();

#ifdef __cplusplus
}
#endif // __cplusplus
