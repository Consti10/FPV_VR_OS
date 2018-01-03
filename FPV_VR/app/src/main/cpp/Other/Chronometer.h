//
// Created by Constantin on 22.10.2017.
//

#ifndef OSDTESTER_CHRONOMETER_H
#define OSDTESTER_CHRONOMETER_H


#include <string>
#include "../Helper/Time.h"

class Chronometer {
public:
    Chronometer(std::string name);
    ~Chronometer();
    void start();
    void stop();
    std::string latestDeltaToString();
    int64_t getAvg();
    void printAvg();
    void reset();
private:
    std::string mName;
    uint64_t lastDeltaUS;
    uint64_t timeSumUS;
    int timeCount;
    uint64_t startTsUS;
};


#endif //OSDTESTER_CHRONOMETER_H
