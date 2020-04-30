//
// Created by Constantin on 6/19/2018.
//

#ifndef FPV_VR_POSITIONDEBUG_H
#define FPV_VR_POSITIONDEBUG_H

#include <GLProgramVC.h>
#include <GLBufferHelper.hpp>
#include <ColoredGeometry.hpp>
#include "IPositionable.hpp"

#define DISABLE_DEBUG_DRAW

class PositionDebug{
private:
#ifndef DISABLE_DEBUG_DRAW
    VertexBuffer vertexBuffer;
    const GLProgramVC& mGLRenderGeometry;
#endif
    const bool mEnableDebugDraw;
    const TrueColor mColor;
    static constexpr const int TESSELATION=6;
public:
    PositionDebug(const GLProgramVC& glRenderGeometry,const int whichColor,const bool enable):
#ifndef DISABLE_DEBUG_DRAW
            mGLRenderGeometry(glRenderGeometry),
#endif
            mEnableDebugDraw(true),mColor(getDebugColor(whichColor)){
#ifndef DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            vertexBuffer.initializeGL();
        }
#endif
    };
    void setWorldPositionDebug(const IPositionable::Rect2D pos){
        setWorldPositionDebug(pos.mX,pos.mY,pos.mZ,pos.mWidth,pos.mHeight);
    }
    void setWorldPositionDebug(float x, float y, float z, float width, float height){
#ifndef  DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            std::vector<GLProgramVC::Vertex> buff(6*TESSELATION*TESSELATION);
            ColoredGeometry::makeTesselatedColoredRect(buff.data(),TESSELATION,glm::vec3(x, y, z), glm::vec3(width, 0, 0), glm::vec3(0, height, 0),mColor);
            vertexBuffer.uploadGL(buff);
        }
#endif
        int i = 0;
    };
    const void drawGLDebug(const glm::mat4x4 ViewM,const glm::mat4x4 ProjM) const{
#ifndef  DISABLE_DEBUG_DRAW
        if(mEnableDebugDraw){
            mGLRenderGeometry.beforeDraw(vertexBuffer.vertexB);
            mGLRenderGeometry.draw(glm::value_ptr(ViewM), glm::value_ptr(ProjM), 0,vertexBuffer.nVertices,GL_TRIANGLES);
            mGLRenderGeometry.afterDraw();
        }
#endif
    }
private:
    static TrueColor getDebugColor(const int debugColorI){
        TrueColor color;
        switch(debugColorI){
            case 0:color=TrueColor2::RED;break; //red
            case 1:color=TrueColor2::GREEN;break; //green
            case 2:color=TrueColor2::BLUE;break; //blue
            case 3:color=TrueColor2::YELLOW;break; //yellow
            case 4:color=TrueColor(1,0.5f,0,0.2f);break; //orange
            case 5:color=TrueColor(0.5f,0.5f,0.5f,0.2f);break; //grey
            case 6:color=TrueColor(1,1,1,0.2f);break; //white
            case 7:color=TrueColor(0,0,0,0.1f);break; //black
            default:color=TrueColor(1,1,1,0.2f);break; //white
        }
        return color;
    }
};
#endif //FPV_VR_POSITIONDEBUG_H
