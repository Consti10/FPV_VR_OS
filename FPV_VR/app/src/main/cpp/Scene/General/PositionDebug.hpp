//
// Created by Constantin on 6/19/2018.
//

#ifndef FPV_VR_POSITIONDEBUG_H
#define FPV_VR_POSITIONDEBUG_H

#include <GLProgramVC.h>
#include "IPositionable.hpp"

#define DISABLE_DEBUG_DRAW

class PositionDebug{
private:
#ifndef DISABLE_DEBUG_DRAW
    GLuint mGLDebugB[1];
    const GLProgramVC& mGLRenderGeometry;
#endif
    const bool mEnableDebugDraw;
    const glm::vec4 mColor;
    const int tesselation=6;
public:
    PositionDebug(const GLProgramVC& glRenderGeometry,const int whichColor,const bool enable):
#ifndef DISABLE_DEBUG_DRAW
            mGLRenderGeometry(glRenderGeometry),
#endif
            mEnableDebugDraw(enable),mColor(getDebugColor(whichColor)){
#ifndef DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            glGenBuffers(1,mGLDebugB);
        }
#endif
    };
    void setWorldPositionDebug(const IPositionable::Rect2D pos){
        setWorldPositionDebug(pos.mX,pos.mY,pos.mZ,pos.mWidth,pos.mHeight);
    }
    void setWorldPositionDebug(float x, float y, float z, float width, float height){
#ifndef  DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            GLProgramVC::Vertex buff[6 * tesselation * tesselation];
            //makeTesselatedColoredRect(buff, 0,tesselation, glm::vec3(x, y, z), glm::vec3(width, 0, 0), glm::vec3(0, height, 0),
            //                          mColor);
            GeometryHelperVC::makeTesselatedColoredRect(buff,tesselation,glm::vec3(x, y, z), glm::vec3(width, 0, 0), glm::vec3(0, height, 0),mColor);
            glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(buff),
                         buff, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
#endif
        int i = 0;
    };
    const void drawGLDebug(const glm::mat4x4 ViewM,const glm::mat4x4 ProjM) const{
#ifndef  DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            mGLRenderGeometry.beforeDraw(mGLDebugB[0]);
            mGLRenderGeometry.draw(glm::value_ptr(ViewM), glm::value_ptr(ProjM), 0, 3 * 2 * tesselation * tesselation,GL_TRIANGLES);
            mGLRenderGeometry.afterDraw();
        }
#endif
    }
private:
    static glm::vec4 getDebugColor(const int debugColorI){
        glm::vec4 color;
        switch(debugColorI){
            case 0:color=glm::vec4(1,0,0,0.2f);break; //red
            case 1:color=glm::vec4(0,1,0,0.2f);break; //green
            case 2:color=glm::vec4(0,0,1,0.2f);break; //blue
            case 3:color=glm::vec4(1,1,0,0.2f);break; //yellow
            case 4:color=glm::vec4(1,0.5f,0,0.2f);break; //orange
            case 5:color=glm::vec4(0.5f,0.5f,0.5f,0.2f);break; //grey
            case 6:color=glm::vec4(1,1,1,0.2f);break; //white
            case 7:color=glm::vec4(0,0,0,0.1f);break; //black
            default:color=glm::vec4(1,1,1,0.2f);break; //white
        }
        return color;
    }
};
#endif //FPV_VR_POSITIONDEBUG_H
