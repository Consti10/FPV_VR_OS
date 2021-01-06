//
// Created by Constantin on 6/19/2018.
//

#ifndef FPV_VR_POSITIONABLE_H
#define FPV_VR_POSITIONABLE_H

#include <GLProgramVC.h>
#include <GLBuffer.hpp>
#include <ColoredGeometry.hpp>

// Used to place the OSD elements in world space.
// Since OSD elements are only in 2D space, this has no "Z" value
class IPositionable{
public:
    struct Rect2D{
        Rect2D(const float x,const float y,const float width,const float height):
        mX(x),mY(y),mWidth(width),mHeight(height){}
        const float mX;
        const float mY;
        const float mWidth;
        const float mHeight;
    };
    struct Rect2D_{
        float X;
        float Y;
        float Width;
        float Height;
    };
public:
    void setWorldPosition(const float x,const float y,const float width,const float height){
        mX=x;
        mY=y;
        mWidth=width;
        mHeight=height;
        setupPosition();
    };
    void setWorldPosition(const Rect2D& rect2D){
        mX=rect2D.mX;
        mY=rect2D.mY;
        mWidth=rect2D.mWidth;
        mHeight=rect2D.mHeight;
        setupPosition();
    }
    void debug(const GLProgramVC& glProgramVc,const glm::mat4 ViewM,const glm::mat4 ProjM){
        auto tmp=ColoredGeometry::makeColoredRectangle({mX,mY,0},mWidth,mHeight,TrueColor2::GREEN);
        mGLBuffDebug.uploadGL(tmp);
        glProgramVc.beforeDraw(mGLBuffDebug);
        glProgramVc.draw(ViewM,ProjM,0,mGLBuffDebug.getCount(),GL_TRIANGLES);
        glProgramVc.afterDraw();
    }
protected:
    float mX,mY;
    float mWidth,mHeight;
protected:
    virtual void setupPosition()=0;
private:
    GLBuffer<ColoredVertex> mGLBuffDebug;
};
#endif //FPV_VR_POSITIONABLE_H
