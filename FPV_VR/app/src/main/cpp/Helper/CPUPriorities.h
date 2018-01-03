//
// Created by Constantin on 27.10.2017.
//

#ifndef OSDTESTER_CPUPRIORITIES_H
#define OSDTESTER_CPUPRIORITIES_H

#include <sys/resource.h>
#include <unistd.h>
#include <android/log.h>

#define TAGCPUPrio "CPUPrio"
#define LOGVCPUPrio(...) __android_log_print(ANDROID_LOG_VERBOSE, TAGCPUPrio, __VA_ARGS__)
/*ANDROID_PRIORITY_LOWEST         =  19,

 use for background tasks
        ANDROID_PRIORITY_BACKGROUND     =  10,

 most threads run at normal priority
        ANDROID_PRIORITY_NORMAL         =   0,

 threads currently running a UI that the user is interacting with
        ANDROID_PRIORITY_FOREGROUND     =  -2,

 the main UI thread has a slightly more favorable priority
        ANDROID_PRIORITY_DISPLAY        =  -4,
 ui service treads might want to run at a urgent display (uncommon)
        ANDROID_PRIORITY_URGENT_DISPLAY =  -8,

 all normal audio threads
        ANDROID_PRIORITY_AUDIO          = -16,

 service audio threads (uncommon)
        ANDROID_PRIORITY_URGENT_AUDIO   = -19,

 should never be used in practice. regular process might not
 be allowed to use this level
        ANDROID_PRIORITY_HIGHEST        = -20,

        ANDROID_PRIORITY_DEFAULT        = ANDROID_PRIORITY_NORMAL,
        ANDROID_PRIORITY_MORE_FAVORABLE = -1,
        ANDROID_PRIORITY_LESS_FAVORABLE = +1,*/

constexpr int CPU_PRIORITY_GLRENDERER_STEREO_FB=-19; //This one needs a whole CPU core all the time
constexpr int CPU_PRIORITY_GLRENDERER_STEREO=-16;

constexpr int CPU_PRIORITY_UDPRECEIVER_VIDEO=-16;
constexpr int CPU_PRIORITY_DECODER_OUTPUT=-16;

constexpr int CPU_PRIORITY_GLRENDERER_MONO=-8;

constexpr int CPU_PRIORITY_UDPRECEIVER_TELEMETRY=-2;

constexpr int CPU_PRIORITY_TEXTELEMENTS_GLREFRESH=0;


static const void setCPUPriority(const int priority,const string caller){
    int which = PRIO_PROCESS;
    auto pid = getpid();
    int ret = setpriority(which, pid, priority);
    if(ret!=0){
        LOGVCPUPrio("Error set thread priority to:%d",priority);
    }
    pid = getpid();
    ret = getpriority(which, pid);
    LOGVCPUPrio("Priority set to %d in %s",ret,caller.c_str());
}
static const void printCPUPriority(const string caller){
    int which = PRIO_PROCESS;
    auto pid = getpid();
    pid = getpid();
    int ret = getpriority(which, pid);
    LOGVCPUPrio("Priority set to %d in %s",ret,caller.c_str());
}
#endif //OSDTESTER_CPUPRIORITIES_H
