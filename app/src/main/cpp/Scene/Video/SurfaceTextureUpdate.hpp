//
// Created by geier on 31/01/2020.
//

#ifndef FPV_VR_OS_SURFACETEXTUREUPDATE_HPP
#define FPV_VR_OS_SURFACETEXTUREUPDATE_HPP

//It makes absolutely no sense to call java code via ndk that is then calling ndk code again - but when building
//for pre-android 9, there is no other way around
//If api<28 we have to use java to update the surface texture
//#include <android/api-level.h>
//#if __ANDROID_API__< __ANDROID_API_P__ //28
#define FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
//#endif

#include <jni.h>
#include <android/surface_texture.h>
#ifndef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
#include <android/surface_texture_jni.h>
#endif

//Only SuperSync needs this one, else we can call updateTexImage() in java
//Helper for calling the java updateTexImage() method from cpp code

class SurfaceTextureUpdate {
private:
    ASurfaceTexture* mSurfaceTexture;//Only valid hen not using FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
    jobject localRefSurfaceTexture;
    jmethodID updateTexImageMethodId;
    jmethodID getTimestampMethodId;
public:
    void initUpdateTexImageJAVA(JNIEnv *env, jobject obj,jobject surfaceTexture) {
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
    };

    void deleteUpdateTexImageJAVA(JNIEnv *env, jobject obj) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
        env->DeleteGlobalRef(localRefSurfaceTexture);
#else
        ASurfaceTexture_release(mSurfaceTexture);
#endif
    };

    void updateTexImageJAVA(JNIEnv* env) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
        env->CallVoidMethod( localRefSurfaceTexture, updateTexImageMethodId );
#else
        ASurfaceTexture_updateTexImage(mSurfaceTexture);
#endif
    };
};


#endif //FPV_VR_OS_SURFACETEXTUREUPDATE_HPP
