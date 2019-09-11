//
// Created by Constantin on 1/21/2019.
//

#ifndef FPV_VR_AGLRENDERER_H
#define FPV_VR_AGLRENDERER_H

#include <jni.h>

/*************************************************************************
 * Connect the java and cpp GLRenderer and
 * add debugging functionality: onSurfaceCreated,onSurfaceChanged and onDrawFrame can be logged
 *************************************************************************/

class IGLRenderer {
public:
    IGLRenderer()= default;
    /**
     * Every scene has a onSurfaceCreated function
     * Has to be called externally (from java) from the OpenGL thread
     */
    void OnSurfaceCreated(JNIEnv * env,jobject androidContext,jint optionalVideoTexture);
    /**
     * Every scene has a onSurfaceChanged function
     * Has to be called externally (from java) from the OpenGL thread
     */
    void OnSurfaceChanged(int width, int height);;
    /**
     * Every scene has a draw function
     * Has to be called externally (from java) from the OpenGL thread
     */
    void OnDrawFrame();
protected:
    //every scene begins with onSurfaceCreated
    virtual void onSurfaceCreated(JNIEnv * env,jobject obj,jint optionalVideoTexture)=0;
    //Every scene has to react to changes in the surface size
    virtual void onSurfaceChanged(int width, int height)=0;
    //Every scene implements either a onDrawFrame function
    //or the enter/exit SuperSync equivalents
    virtual void onDrawFrame()=0;
};



#endif //FPV_VR_AGLRENDERER_H
