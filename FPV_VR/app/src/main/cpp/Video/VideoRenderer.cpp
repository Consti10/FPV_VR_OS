
#include "VideoRenderer.h"
#include "../Helper/GLHelper.h"
#include <../Helper/GeometryHelper.h>

#define TAG "VideoRenderer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

VideoRenderer::VideoRenderer(GLRenderColor *glRenderColor,GLRenderTextureExternal *glRenderTexEx,bool eTesselation) {
#ifdef DEBUG_POSITION
    glGenBuffers(1,mGLDebugB);
#endif
    mGLRenderColor=glRenderColor;
    mGLRenderTexEx=glRenderTexEx;
    enableTesselation=eTesselation;
    glGenBuffers(1,mGLBuffer);
}

void VideoRenderer::setWorldPosition(float videoX, float videoY, float videoZ, float videoW,float videoH) {
#ifdef DEBUG_POSITION
    float debug[7*6];
    makeColoredRect(debug,0,glm::vec3(videoX,videoY,videoZ),glm::vec3(videoW,0,0),glm::vec3(0,videoH,0),0,1,1,1);
    glBindBuffer(GL_ARRAY_BUFFER, mGLDebugB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debug),
                 debug, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    if(enableTesselation){
        float tmp[5*6*TesselationFactor*TesselationFactor];
        makeTesselatedVideoCanvas(tmp,0,glm::vec3(videoX,videoY,videoZ),videoW,videoH,TesselationFactor);
        glBindBuffer(GL_ARRAY_BUFFER, mGLBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                     tmp, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        nVertices=2*3*TesselationFactor*TesselationFactor;
    }else{
        float tmp[5*6];
        makeVideoCanvas(tmp,0,glm::vec3(videoX,videoY,videoZ),videoW,videoH);
        glBindBuffer(GL_ARRAY_BUFFER, mGLBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tmp),
                     tmp, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        nVertices=2*3;
    }
}

void VideoRenderer::drawGL(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
#ifdef DEBUG_POSITION
    mGLRenderColor->beforeDraw(mGLDebugB[0]);
    mGLRenderColor->draw(ViewM,ProjM,0,2*3);
    mGLRenderColor->afterDraw();
#endif
    mGLRenderTexEx->beforeDraw(mGLBuffer[0]);
    mGLRenderTexEx->draw(ViewM,ProjM,0,nVertices);
    mGLRenderTexEx->afterDraw();
    checkGlError("VideoRenderer::drawGL");
}

void VideoRenderer::initUpdateTexImageJAVA(JNIEnv *env, jobject obj,jobject surfaceTexture) {
    static const char * className = "android/graphics/SurfaceTexture";
    const jclass surfaceTextureClass = env->FindClass(className);
    LOGV("SurfaceTextureStart");
    if ( surfaceTextureClass == 0 ) {
        LOGV( "FindClass( %s ) failed", className );
    }
    // find the constructor that takes an int
    const jmethodID constructor = env->GetMethodID( surfaceTextureClass, "<init>", "(I)V" );
    if ( constructor == 0 ) {
        LOGV( "GetMethodID( <init> ) failed" );
    }
    localRefSurfaceTexture = env->NewGlobalRef( surfaceTexture);
    if ( localRefSurfaceTexture == 0 ) {
        LOGV( "NewGlobalRef() failed" );
    }
    // Now that we have a globalRef, we can free the localRef
    env->DeleteLocalRef( surfaceTexture );
    //get the java methods that can be called with a valid surfaceTexture instance and JNI env
    updateTexImageMethodId = env->GetMethodID( surfaceTextureClass, "updateTexImage", "()V" );
    if ( !updateTexImageMethodId ) {
        LOGV("couldn't get updateTexImageMethodId" );
    }
    getTimestampMethodId = env->GetMethodID( surfaceTextureClass, "getTimestamp", "()J" );
    if ( !getTimestampMethodId ) {
        LOGV( "couldn't get getTimestampMethodId" );
    }
}
void VideoRenderer::deleteUpdateTexImageJAVA(JNIEnv *env, jobject obj) {
    env->DeleteGlobalRef(localRefSurfaceTexture);
}

void VideoRenderer::updateTexImageJAVA(JNIEnv* env) {
    env->CallVoidMethod( localRefSurfaceTexture, updateTexImageMethodId );
}


