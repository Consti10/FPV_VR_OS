//
// Created by Constantin on 25.11.2017.
//

#ifndef FPV_VR_FRAMECPUCHRONOMETER_H
#define FPV_VR_FRAMECPUCHRONOMETER_H


#include <string>
#include <vector>

class FrameCPUChronometer {
public:
    FrameCPUChronometer(std::vector<std::string> timestampNames);
    void start(bool whichEye);
    void setTimestamp(bool whichEye,int whichTimestamp);
    void stop(bool whichEye);
    void print();
private:
    class FrameCPUTimestamp{
    public:
        FrameCPUTimestamp(std::string name){
            this->name=name;
            reset();
        }
        void reset(){
            leftEyeLastDelta=0;
            rightEyeLastDelta=0;
            leftEyeDeltaSumUs=0;
            leftEyeDeltaC=0;
            rightEyeDeltaSumUs=0;
            rightEyeDeltaC=0;
            leftEyeLastStop=0;
            rightEyeLastStop=0;
        }
        std::string name;
        int64_t leftEyeLastDelta;
        int64_t rightEyeLastDelta;
        uint64_t leftEyeDeltaSumUs;
        uint64_t leftEyeDeltaC;
        uint64_t rightEyeDeltaSumUs;
        uint64_t rightEyeDeltaC;
        int64_t leftEyeLastStop;
        int64_t rightEyeLastStop;
    };
    std::vector<FrameCPUTimestamp*> mFrameCPUTimestamps;
    int64_t leftEyeLastStart=0,rightEyeLastStart=0;
    int64_t leftEyeLastDelta=0,rightEyeLastDelta=0;
    uint64_t leftEyeDeltaSumUs=0,leftEyeDeltaC=0;
    uint64_t rightEyeDeltaSumUs=0,rightEyeDeltaC=0;

    void resetTS();
    int64_t lastAvgLogMS;
};


#endif //FPV_VR_FRAMECPUCHRONOMETER_H
