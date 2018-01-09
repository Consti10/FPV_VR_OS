//
// Created by Constantin on 05.01.2018.
//

#ifndef FPV_VR_IRENDEREREGL14_H
#define FPV_VR_IRENDEREREGL14_H

class IRendererEGL14{
    virtual void onGLSurfaceCreated()=0;
    virtual void onGLSurfaceChanged()=0;
    virtual void onDrawFrame()=0;
    virtual void onGLSurfaceDestroyed()=0;
};
#endif //FPV_VR_IRENDEREREGL14_H
