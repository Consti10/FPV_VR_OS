
//It is complete bullshit to call java code via ndk that is then calling ndk code again - but when building
//for pre-android 9, there is no other way around
#define FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE

#include <Color/Color.hpp>
#include <GeometryBuilder/ColoredGeometry.hpp>
#include <GeometryBuilder/TexturedGeometry.hpp>
#include <GeometryBuilder/EquirectangularSphere.hpp>
#include "VideoRenderer.h"
#include "Helper/GLHelper.hpp"
#include "Helper/GLBufferHelper.hpp"

#ifndef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#endif


constexpr auto TAG="VideoRenderer";
#define LOGD1(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

VideoRenderer::VideoRenderer(VIDEO_RENDERING_MODE mode,const GLuint videoTexture,const GLProgramVC& glRenderGeometry,GLProgramTexture *glRenderTexEx):
mVideoTexture(videoTexture),
mMode(mode),mPositionDebug(glRenderGeometry,6, false),mGLRenderGeometry(glRenderGeometry){
    mGLRenderTexEx=glRenderTexEx;
    switch (mMode){
        case RM_NORMAL:
            mVideoCanvasB.initializeGL();
            break;
        case RM_STEREO:
            mVideoCanvasLeftEyeB.initializeGL();
            mVideoCanvasRightEyeB.initializeGL();
            break;
        case RM_360_EQUIRECTANGULAR:
            mEquirectangularSphereB.initializeGL();
            break;
    }
}

void VideoRenderer::updatePosition(const glm::vec3& lowerLeftCorner,const float width,const float height,
        const int optionalVideoWidthPx,const int optionalVideoHeightPx) {
    if(mMode==RM_NORMAL){
        const auto vid0=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    1.0f);
        mVideoCanvasB.initializeAndUploadGL(vid0.vertices,vid0.indices);
    }else if(mMode==RM_STEREO){
        const auto vid1=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                    width,height, TESSELATION_FACTOR, 0.0f,
                                                                    0.5f);
        mVideoCanvasLeftEyeB.initializeAndUploadGL(vid1.vertices,vid1.indices);
        const auto vid2=TexturedGeometry::makeTesselatedVideoCanvas(lowerLeftCorner,
                                                                     width,height, TESSELATION_FACTOR, 0.5f,
                                                                     0.5f);
        mVideoCanvasRightEyeB.initializeAndUploadGL(vid2.vertices,vid2.indices);
    }else if(mMode==RM_360_EQUIRECTANGULAR){
        //We need to recalculate the sphere u,v coordinates when the video ratio changes
        EquirectangularSphere::uploadSphereGL(mEquirectangularSphereB,optionalVideoWidthPx,optionalVideoHeightPx);
    }
}

void VideoRenderer::drawVideoCanvas(glm::mat4x4 ViewM, glm::mat4x4 ProjM, bool leftEye) {
    if(mMode==RM_360_EQUIRECTANGULAR){
        drawVideoCanvas360(ViewM,ProjM);
    }else if(mMode==RM_NORMAL || mMode==RM_STEREO){
        const auto buff=mMode==RM_NORMAL ? mVideoCanvasB : leftEye ? mVideoCanvasLeftEyeB : mVideoCanvasRightEyeB;
        mGLRenderTexEx->beforeDraw(buff.vertexB,mVideoTexture);
        mGLRenderTexEx->drawIndexed(buff.indexB,ViewM,ProjM,0,buff.nIndices,GL_TRIANGLES);
        mGLRenderTexEx->afterDraw();
    }
    //We render the debug rectangle after the other one such that it always appears when enabled (overdraw)
    mPositionDebug.drawGLDebug(ViewM,ProjM);
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas");
}

void VideoRenderer::drawVideoCanvas360(glm::mat4x4 ViewM, glm::mat4x4 ProjM) {
    if(mMode!=VIDEO_RENDERING_MODE::RM_360_EQUIRECTANGULAR){
        throw "mMode!=VIDEO_RENDERING_MODE::Degree360";
    }
    const float scale=200.0f;
    glm::mat4 scaleM=glm::scale(glm::vec3(scale,scale,scale));
    mGLRenderTexEx->beforeDraw(mEquirectangularSphereB.vertexB,mVideoTexture);
    mGLRenderTexEx->drawIndexed(mEquirectangularSphereB.indexB,ViewM*scaleM,ProjM,0,mEquirectangularSphereB.nIndices,GL_TRIANGLE_STRIP);
    mGLRenderTexEx->afterDraw();
    GLHelper::checkGlError("VideoRenderer::drawVideoCanvas360");
}

void VideoRenderer::initUpdateTexImageJAVA(JNIEnv *env, jobject obj,jobject surfaceTexture) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
    jclass jcSurfaceTexture = env->FindClass("android/graphics/SurfaceTexture");
    LOGD("SurfaceTextureStart");
    if ( jcSurfaceTexture == nullptr ) {
        LOGD( "FindClass( SurfaceTexture ) failed");
    }
    // find the constructor that takes an int
    jmethodID constructor = env->GetMethodID( jcSurfaceTexture, "<init>", "(I)V" );
    if ( constructor == nullptr) {
        LOGD( "GetMethodID( <init> ) failed" );
    }
    localRefSurfaceTexture = env->NewGlobalRef( surfaceTexture);
    if ( localRefSurfaceTexture == nullptr ) {
        LOGD( "NewGlobalRef() failed" );
    }
    // Now that we have a globalRef, we can free the localRef
    env->DeleteLocalRef( surfaceTexture );
    //get the java methods that can be called with a valid surfaceTexture instance and JNI env
    updateTexImageMethodId = env->GetMethodID( jcSurfaceTexture, "updateTexImage", "()V" );
    if ( !updateTexImageMethodId ) {
        LOGD("couldn't get updateTexImageMethodId" );
    }
    getTimestampMethodId = env->GetMethodID( jcSurfaceTexture, "getTimestamp", "()J" );
    if ( !getTimestampMethodId ) {
        LOGD( "couldn't get getTimestampMethodId" );
    }
#else
    mSurfaceTexture=ASurfaceTexture_fromSurfaceTexture(env,surfaceTexture);
#endif
}

void VideoRenderer::deleteUpdateTexImageJAVA(JNIEnv *env, jobject obj) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
    env->DeleteGlobalRef(localRefSurfaceTexture);
#endif
}

void VideoRenderer::updateTexImageJAVA(JNIEnv* env) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
    env->CallVoidMethod( localRefSurfaceTexture, updateTexImageMethodId );
#else
    ASurfaceTexture_updateTexImage(mSurfaceTexture);
#endif
}




