//
// Created by Constantin on 22.10.2017.
//

#include <sstream>
#include "Chronometer.h"
#include "jni.h"
#include "android/log.h"

#define LOGT(...) __android_log_print(ANDROID_LOG_VERBOSE,"", __VA_ARGS__)


Chronometer::Chronometer(std::string name) {
    mName=name;
    startTsUS=getTimeUS();
    reset();
}

void Chronometer::start() {
    startTsUS=getTimeUS();
}

void Chronometer::stop() {
    uint64_t ts=getTimeUS();
    lastDeltaUS=ts-startTsUS;
    timeSumUS+=lastDeltaUS;
    timeCount++;
}

std::string Chronometer::latestDeltaToString() {
    std::ostringstream oss;
    oss<<mName<<":"<<lastDeltaUS/1000;
    return oss.str();
}

void Chronometer::printAvg() {
    int64_t currAvgT=timeSumUS/timeCount;
    LOGT("Avg: %s:%f",mName.c_str(),(float)(((double)currAvgT)*0.001f));
}

void Chronometer::reset() {
    lastDeltaUS=0;
    timeSumUS=0;
    timeCount=0;
}

int64_t Chronometer::getAvg() {
    if(timeCount>0){
        return (timeSumUS/timeCount);
    }
    return 0;
}

Chronometer::~Chronometer() {
    LOGT("Chronometer destroyed %s",mName.c_str());
}

