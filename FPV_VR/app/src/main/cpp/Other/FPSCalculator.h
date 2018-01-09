//
// Created by Constantin on 06.01.2018.
//

#ifndef FPV_VR_FPSCALCULATOR_H
#define FPV_VR_FPSCALCULATOR_H


#include <cstdint>
#include <string>
#include "../Helper/Time.h"

class FPSCalculator {
private:
    std::string NAME;
    int INTERVAL_MS;
    int64_t lastFPSCalculation=0;
    double ticksSinceLastFPSCalculation=0;
    double currFPS;
public:
    FPSCalculator(std::string name,int printIntervalMS);
    void tick();
    void print();
    float getCurrentFPS();
};


#endif //FPV_VR_FPSCALCULATOR_H
