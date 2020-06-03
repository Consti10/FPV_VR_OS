//
// Created by geier on 31/01/2020.
//

#ifndef FPV_VR_OS_SURFACETEXTUREUPDATE_HPP
#define FPV_VR_OS_SURFACETEXTUREUPDATE_HPP

//It makes absolutely no sense to call java code via ndk that is then calling ndk code again - but when building
//for pre-android 9 (api 28), there is no other way around
//If api<28 we have to use java to update the surface texture
//#include <android/api-level.h>
//#if __ANDROID_API__< __ANDROID_API_P__ //28
#define FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
//#endif

#include <jni.h>
#include <android/surface_texture_jni.h>
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
#include <android/surface_texture.h>
#else
#include <android/surface_texture_jni.h>
#include <android/surface_texture.h>

#undef __ANDROID_API__
#define __ANDROID_API__ 28
#undef __INTRODUCED_IN
#define __INTRODUCED_IN(api_level) __attribute__((annotate("introduced_in=21")))

#include <android/surface_texture_jni.h>
#include <android/surface_texture.h>

#undef __ANDROID_API__
#undef __INTRODUCED_IN
#define __ANDROID_API__ 21
#define __INTRODUCED_IN(api_level) __attribute__((annotate("introduced_in=" #api_level)))

#endif //FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE

#include <optional>
#include <chrono>

//Helper for calling the ASurfaceTexture_XXX method with a fallback for minApi<28
// 03.06.2020 confirmed that the ASurfaceTexture_XXX methods call native code directly (not java)

class SurfaceTextureUpdate {
private:
    jmethodID updateTexImageMethodId;
    jmethodID getTimestampMethodId;
    //Only valid hen not using FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
    ASurfaceTexture* mSurfaceTexture;
    // set later (not in constructor)
    jobject weakGlobalRefSurfaceTexture;
    //JNIEnv* env1;
public:
    // look up all the method ids
    SurfaceTextureUpdate(JNIEnv* env){
        jclass jcSurfaceTexture = env->FindClass("android/graphics/SurfaceTexture");
        MLOGD<<"SurfaceTextureStart";
        if ( jcSurfaceTexture == nullptr ) {
            MLOGD<< "FindClass( SurfaceTexture ) failed";
        }
        // find the constructor that takes an int
        jmethodID constructor = env->GetMethodID( jcSurfaceTexture, "<init>", "(I)V" );
        if ( constructor == nullptr) {
            MLOGD<<"GetMethodID( <init> ) failed";
        }
        //get the java methods that can be called with a valid surfaceTexture instance and JNI env
        updateTexImageMethodId = env->GetMethodID( jcSurfaceTexture, "updateTexImage", "()V" );
        if ( !updateTexImageMethodId ) {
            MLOGD<<"couldn't get updateTexImageMethodId";
        }
        getTimestampMethodId = env->GetMethodID( jcSurfaceTexture, "getTimestamp", "()J" );
        if ( !getTimestampMethodId ) {
            MLOGD<<"couldn't get getTimestampMethodId";
        }
    }
    /**
     * set the wrapped SurfaceTexture. this has to be delayed (cannot be done in constructor)
     * @param surfaceTexture1 when nullptr delete previosuly aquired reference, else create new reference
     */
    void setSurfaceTexture(JNIEnv* env,jobject surfaceTexture1){
        //env1=env;
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
        if(surfaceTexture1==nullptr){
            assert(weakGlobalRefSurfaceTexture!=nullptr);
            env->DeleteWeakGlobalRef(weakGlobalRefSurfaceTexture);
            weakGlobalRefSurfaceTexture=nullptr;
        }else{
            weakGlobalRefSurfaceTexture = env->NewWeakGlobalRef(surfaceTexture1);
        }
#else
        mSurfaceTexture=ASurfaceTexture_fromSurfaceTexture(env,surfaceTexture);
#endif
    }
    void updateTexImageJAVA(JNIEnv* env) {
#ifdef FPV_VR_USE_JAVA_FOR_SURFACE_TEXTURE_UPDATE
        env->CallVoidMethod(weakGlobalRefSurfaceTexture, updateTexImageMethodId );
#else
        ASurfaceTexture_updateTexImage(mSurfaceTexture);
#endif
    };
    long getTimestamp(JNIEnv* env){
        return env->CallLongMethod(weakGlobalRefSurfaceTexture, getTimestampMethodId);
    }
    // on success, returns delay between producer enqueueing buffer and consumer (gl) dequeueing it
    // on failure (no new image available) returns std::nullopt
    std::optional<std::chrono::steady_clock::duration> updateAndCheck(JNIEnv* env){
        const long oldTimestamp=getTimestamp(env);
        updateTexImageJAVA(env);
        const long newTimestamp=getTimestamp(env);
        if(newTimestamp!=oldTimestamp){
            const auto delay=std::chrono::steady_clock::now().time_since_epoch()-std::chrono::nanoseconds(newTimestamp);
            return delay;
        }
        return std::nullopt;
    }
};


#endif //FPV_VR_OS_SURFACETEXTUREUPDATE_HPP
