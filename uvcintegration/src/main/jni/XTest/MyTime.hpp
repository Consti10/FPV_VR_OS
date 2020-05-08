//
// Created by geier on 06/05/2020.
//

#ifndef UVCCAMERA_TIME_H
#define UVCCAMERA_TIME_H

#include <stdio.h>
#include <time.h>

static uint64_t GetTicksNanos()
{
    // Choreographer vsync timestamp is based on.
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);

    if (status != 0)
    {
        //CLOGD("clock_gettime status=%d", status );
    }
    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t (tp.tv_nsec);
    return result;
}

#endif //UVCCAMERA_TIME_H
