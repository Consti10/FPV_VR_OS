//
// Created by Constantin on 6/19/2018.
//

#ifndef FPV_VR_IUPDATEABLE_H
#define FPV_VR_IUPDATEABLE_H

#include <utility>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include "Helper/GLHelper.hpp"

/**
 *  cpp Interface. Similar to IUpdateable
 */

//#define DEBUG_PRINT_TIME
#define CHECK_GL_ERROR

class IUpdateable{
protected:
    virtual void updateGL()=0;
public:
    const std::string NAME;
    explicit IUpdateable(std::string name="Unnamed"):NAME(std::move(name)){}
public:
    /**
    * @return the time delta, for debugging, in microseconds,if Time measuring is enabled
    * Else return 0
    */
    int updateGLBase(){
#ifdef DEBUG_PRINT_TIME
        auto before = std::chrono::steady_clock::now();
#endif
        updateGL();
#ifdef CHECK_GL_ERROR
        GLHelper::checkGlError("IUpdateable:"+NAME);
#endif
#ifdef DEBUG_PRINT_TIME
        auto after = std::chrono::steady_clock::now();
        return (int)std::chrono::duration_cast<std::chrono::microseconds> (after-before).count();
#else
        return 0;
#endif
    }
public:
    static void updateAll(const std::vector<IUpdateable*>& updateables){
#ifdef DEBUG_PRINT_TIME
        std::stringstream ss;
        ss<<std::fixed;
        int totalTime=0;
#endif
        for(IUpdateable* updateable:updateables){
            auto time=updateable->updateGLBase();
#ifdef DEBUG_PRINT_TIME
            ss<<updateable->NAME<<": "<<time/1000.0<<"\n";
            totalTime+=time;
#endif
        }
#ifdef DEBUG_PRINT_TIME
        ss<<"Total time:"<<totalTime/1000.0;
        __android_log_print(ANDROID_LOG_DEBUG,"IUpdateable","\n%s",ss.str().c_str());
#endif
    }
private:
    unsigned int cyclicIndex=0;
public:
    unsigned int getCyclicIndex(const unsigned long maxValue){
        cyclicIndex++;
        if(cyclicIndex>maxValue){
            cyclicIndex=0;
        }
        return cyclicIndex;
    }
};

#endif //FPV_VR_IUPDATEABLE_H
