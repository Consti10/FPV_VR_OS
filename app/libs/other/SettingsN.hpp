//
// Created by Constantin on 24.10.2017.
//

#ifndef FPV_VR_SETTINGSN_H
#define FPV_VR_SETTINGSN_H

#include <jni.h>
#include <android/log.h>

class SettingsN {
public:
    SettingsN(SettingsN const &) = delete;
    void operator=(SettingsN const &)= delete;
public:
    SettingsN(JNIEnv *env, jobject androidContext,const char* name){
        this->env=env;
        //Find the 2 java classes we need to make calls with
        jclass jcContext = env->FindClass("android/content/Context");
        jclass jcSharedPreferences = env->FindClass("android/content/SharedPreferences");
        if(jcContext==nullptr || jcSharedPreferences== nullptr){
            __android_log_print(ANDROID_LOG_DEBUG, "SettingsN","Cannot find classes");
        }
        //find the 3 functions we need to get values from an SharedPreferences instance and store the references to them for later use
        jmGetBoolean=env->GetMethodID(jcSharedPreferences,"getBoolean","(Ljava/lang/String;Z)Z");
        jmGetInt=env->GetMethodID(jcSharedPreferences,"getInt","(Ljava/lang/String;I)I");
        jmGetFloat=env->GetMethodID(jcSharedPreferences,"getFloat","(Ljava/lang/String;F)F");

        //create a instance of SharedPreferences and store it in 'joSharedPreferences'
        jmethodID jmGetSharedPreferences=env->GetMethodID(jcContext,"getSharedPreferences","(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
        joSharedPreferences=env->CallObjectMethod(androidContext,jmGetSharedPreferences,env->NewStringUTF(name),MODE_PRIVATE);
    }
    JNIEnv* env;
    jobject joSharedPreferences;
    jmethodID jmGetBoolean;
    jmethodID jmGetInt;
    jmethodID jmGetFloat;
public:
    bool getBoolean(const char* id,bool defaultValue=false)const{
        return (bool)(env->CallBooleanMethod(joSharedPreferences,jmGetBoolean,env->NewStringUTF(id),(jboolean)defaultValue));
    }
    int getInt(const char* id,int defaultValue=0)const{
        return (int)(env->CallIntMethod(joSharedPreferences,jmGetInt,env->NewStringUTF(id),(jint)defaultValue));
    }
    float getFloat(const char* id,float defaultValue=0.0f)const{
        return (float)(env->CallFloatMethod(joSharedPreferences,jmGetFloat,env->NewStringUTF(id),(jfloat)defaultValue));
    }
private:
    static constexpr const int  MODE_PRIVATE = 0; //taken directly from java, assuming this value stays constant in java
};

#endif

