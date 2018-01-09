//
// Created by Constantin on 06.01.2018.
//

#include <android/log.h>
#include "FPSCalculator.h"

#define TAG "FPSCalculator"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

FPSCalculator::FPSCalculator(std::string name, int printIntervalMS) {
    NAME=name;
    INTERVAL_MS=printIntervalMS;
}

void FPSCalculator::tick() {
    ticksSinceLastFPSCalculation++;
    int64_t ts=getTimeMS();
    int64_t deltaMS=ts-lastFPSCalculation;
    if(deltaMS>INTERVAL_MS){
        double exactElapsedSeconds=deltaMS*0.001;
        currFPS=ticksSinceLastFPSCalculation/exactElapsedSeconds;
        ticksSinceLastFPSCalculation=0;
        lastFPSCalculation=ts;
        LOGV("%s:%f",NAME.c_str(),currFPS);
    }
}

float FPSCalculator::getCurrentFPS() {
    return (float)currFPS;
}

