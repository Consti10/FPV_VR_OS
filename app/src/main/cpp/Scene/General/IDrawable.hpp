//
// Created by Constantin on 6/19/2018.
//

#ifndef FPV_VR_DRAWABLE_H
#define FPV_VR_DRAWABLE_H

#include <utility>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include "android/log.h"
#include <glm/mat4x4.hpp>
#include <GLES2/gl2.h>
#include "GLHelper.hpp"

/**
 * cpp Interface for Drawables. Elements that can be drawn using OpenGL should implement this interface such that
 * You can draw all drawables using only one line of code
 * Also adds debugging information (e.g. about the cpu time)
 */


//#define DEBUG_PRINT_TIME
#define CHECK_GL_ERROR

class IDrawable{
protected:
    virtual void drawGL(const glm::mat4& ViewM,const glm::mat4& ProjM)=0;
public:
    const std::string NAME;
    explicit IDrawable(std::string name="Unnamed"):NAME(std::move(name)){}
public:
    /**
* @return the time delta, for debugging, in microseconds,if Time measuring is enabled.
 * Else return 0
*/
    int drawGLBase(glm::mat4 ViewM, glm::mat4 ProjM){
#ifdef DEBUG_PRINT_TIME
        auto before = std::chrono::steady_clock::now();
#endif
        drawGL(ViewM,ProjM);
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
    static void drawAll(const std::vector<IDrawable*>& drawables,glm::mat4 ViewM,glm::mat4 ProjM){
#ifdef DEBUG_PRINT_TIME
        std::stringstream ss;
        ss<<std::fixed;
        int totalTime=0;
#endif
        for(IDrawable* drawable:drawables){
            auto time=drawable->drawGLBase(ViewM,ProjM);
#ifdef DEBUG_PRINT_TIME
            ss<<drawable->NAME<<": "<<time/1000.0<<"\n";
            totalTime+=time;
#endif
        }
#ifdef DEBUG_PRINT_TIME
        ss<<"Total time:"<<totalTime/1000.0;
        __android_log_print(ANDROID_LOG_DEBUG,"IDrawable","\n%s",ss.str().c_str());
#endif
    }
};

#endif //FPV_VR_DRAWABLE_H
