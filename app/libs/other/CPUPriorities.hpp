//
// Created by Constantin on 27.10.2017.
//

#ifndef OSDTESTER_CPUPRIORITIES_H
#define OSDTESTER_CPUPRIORITIES_H

#include <sys/resource.h>
#include <unistd.h>

constexpr int ANDROID_PRIORITY_LOWEST         =  19;

//use for background tasks
constexpr int ANDROID_PRIORITY_BACKGROUND     =  10;

//most threads run at normal priority
constexpr int ANDROID_PRIORITY_NORMAL         =   0;

//threads currently running a UI that the user is interacting with
constexpr int ANDROID_PRIORITY_FOREGROUND     =  -2;

//the main UI thread has a slightly more favorable priority
constexpr int ANDROID_PRIORITY_DISPLAY        =  -4;
//ui service treads might want to run at a urgent display (uncommon)
constexpr int ANDROID_PRIORITY_URGENT_DISPLAY =  -8;

//all normal audio threads
constexpr int ANDROID_PRIORITY_AUDIO          = -16;

//service audio threads (uncommon)
constexpr int ANDROID_PRIORITY_URGENT_AUDIO   = -19;

//should never be used in practice. regular process might not
//be allowed to use this level
constexpr int ANDROID_PRIORITY_HIGHEST        = -20;

/*ANDROID_PRIORITY_DEFAULT        = ANDROID_PRIORITY_NORMAL,
ANDROID_PRIORITY_MORE_FAVORABLE = -1,
ANDROID_PRIORITY_LESS_FAVORABLE = +1,*/

constexpr int CPU_PRIORITY_GLRENDERER_STEREO_FB=-19; //This one needs a whole CPU core all the time anyways
constexpr int CPU_PRIORITY_GLRENDERER_STEREO=-16; //The GL thread also should get 1 whole cpu core

constexpr int CPU_PRIORITY_GLRENDERER_MONO=-2;

constexpr int CPU_PRIORITY_UDPRECEIVER_TELEMETRY=-2; //-4
constexpr int CPU_PRIORITY_UDPSENDER_HEADTRACKING=-2; //-4


static const void setCPUPriority(const int priority,const std::string caller){
    int which = PRIO_PROCESS;
    auto pid = getpid();
    int ret = setpriority(which, (id_t)pid, priority);
    if(ret!=0){
        __android_log_print(ANDROID_LOG_DEBUG, "CPUPrio1","Error set thread priority to:%d in %s",priority,caller.c_str());
    }
    pid = getpid();
    ret = getpriority(which, (id_t)pid);
}

/*static const void printCPUPriority(const std::string caller){
    int which = PRIO_PROCESS;
    auto pid = getpid();
    pid = getpid();
    int ret = getpriority(which, pid);
    __android_log_print(ANDROID_LOG_DEBUG, "CPUPrio", "Priority set to %d in %s",ret,caller.c_str());
}*/

#endif //OSDTESTER_CPUPRIORITIES_H
