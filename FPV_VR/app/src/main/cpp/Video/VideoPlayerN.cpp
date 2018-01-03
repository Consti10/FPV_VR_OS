/*************************************************************************
 * Glue for the VideoPlayer Java class
 * ***********************************************************************/

#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h>
#include <LowLagDecoder.h>
#include <UDPReceiver.h>
#include <TelemetryReceiver.h>
#include "../Helper/CPUPriorities.h"
#include "../SettingsN.h"

LowLagDecoder *mLowLagDecoder=NULL;
UDPReceiver *mVideoReceiver;

float W=1280,H=720;

jobject globalJavaObj;
jclass globalVideoPlayerInstanceClassJAVA;
JavaVM* mJavaVirtualMachine;

void onDataReceivedCallback(uint8_t data[],int data_length){
    if(mLowLagDecoder!=NULL) {
        mLowLagDecoder->parseForNALU(data, data_length, getSystemTimeUS());
    }
}

void onDecoderRatioChangedCallback(int videoW,int videoH){
    LOGV("output format / buffers changed,%dx%d",videoW,videoH);
    //When this callback is invoked, no Java VM is attached to the thread
    JNIEnv* jniENV;
    mJavaVirtualMachine->AttachCurrentThread(&jniENV,NULL);
    jmethodID onVideoRatioChangedJAVA = jniENV->GetMethodID(globalVideoPlayerInstanceClassJAVA, "onDecoderRatioChanged", "(II)V");
    jniENV->CallVoidMethod(globalJavaObj,onVideoRatioChangedJAVA,(jint)videoW,(jint)videoH);
    mJavaVirtualMachine->DetachCurrentThread();
}
void onDecoderFpsChangedCallback(float decFPS){
    //LOGV("Decoder FPS%f",decFPS);
    //When this callback is invoked, no Java VM is attached to the thread
    JNIEnv* jniENV;
    mJavaVirtualMachine->AttachCurrentThread(&jniENV,NULL);
    jmethodID onVideoRatioChangedJAVA = jniENV->GetMethodID(globalVideoPlayerInstanceClassJAVA, "onDecoderFpsChanged", "(F)V");
    jniENV->CallVoidMethod(globalJavaObj,onVideoRatioChangedJAVA,(jfloat)decFPS);
    mJavaVirtualMachine->DetachCurrentThread();
}

void notifyUser(std::string message){
    JNIEnv* jniENV;
    mJavaVirtualMachine->AttachCurrentThread(&jniENV,NULL);
    jstring  messageJ=jniENV->NewStringUTF(message.c_str());
    jmethodID notifyUserJAVA = jniENV->GetMethodID(globalVideoPlayerInstanceClassJAVA, "notifyUser", "(Ljava/lang/String;)V");
    jniENV->CallVoidMethod(globalJavaObj,notifyUserJAVA,messageJ);
    mJavaVirtualMachine->DetachCurrentThread();
}

extern "C"{

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_createDecoder(
        JNIEnv * env, jobject obj,jboolean limitFPS,jobject surface) {
    //mtestTelemetryReceiver=new TelemetryReceiver(S_OSD_ParseLTM,S_LTMPort,S_OSD_ParseFRSKY,S_FRSKYPort,S_OSD_ParseMAVLINK,
                                             //S_MAVLINKPort,S_OSD_ParseRSSI,S_RSSIPort);
    ANativeWindow* window=ANativeWindow_fromSurface(env,surface);
    env->GetJavaVM(&mJavaVirtualMachine);//We need a reference to the JavaVM to attach the callback thread to it
    jclass tmpVideoPlayerInstanceClassJAVA= env->GetObjectClass(obj); //Get the VideoPlayer instance
    globalVideoPlayerInstanceClassJAVA = reinterpret_cast<jclass>(env->NewGlobalRef(tmpVideoPlayerInstanceClassJAVA));
    globalJavaObj = reinterpret_cast<jclass>(env->NewGlobalRef(obj)); //also need a global javaObj
    mLowLagDecoder=new LowLagDecoder(limitFPS,window);
    mLowLagDecoder->registerOnDecoderRatioChangedCallback(onDecoderRatioChangedCallback);
    mLowLagDecoder->registerOnDecoderFpsChangedCallback(onDecoderFpsChangedCallback);
}

//Does return immediately
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_shutdownDecoder(
        JNIEnv * env, jobject obj) {
    mLowLagDecoder->shutdown();
    delete(mLowLagDecoder);
    mLowLagDecoder=NULL;
    env->DeleteGlobalRef(globalJavaObj);
    env->DeleteGlobalRef(globalVideoPlayerInstanceClassJAVA);
}

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_requestShutdown(
        JNIEnv * env, jobject obj) {
    mLowLagDecoder->requestShutdown();
}

JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_waitAndDelete(
        JNIEnv * env, jobject obj) {
    mLowLagDecoder->waitForShutdownAndDelete();
    delete(mLowLagDecoder);
    mLowLagDecoder=NULL;
    env->DeleteGlobalRef(globalJavaObj);
    env->DeleteGlobalRef(globalVideoPlayerInstanceClassJAVA);
}


//This function is only called when the data for the video is coming from a file.
//In this case, the receiving is done via JAVA, and the received bytes are transfered from JAVA to NDK
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_passNALUDataToNative(
        JNIEnv * env, jobject obj,jbyteArray b,jint length) {
    jbyte *arrayP=env->GetByteArrayElements(b,NULL);
    onDataReceivedCallback((uint8_t*)arrayP,length);
    env->ReleaseByteArrayElements(b,arrayP,0);
}


JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_createUDPReceiver(
        JNIEnv * env, jobject obj,jint port) {
    mVideoReceiver=new UDPReceiver((int)port,UDPReceiver::MODE_BLOCKING,onDataReceivedCallback,"VideoPlayerN VideoReceiver",CPU_PRIORITY_UDPRECEIVER_VIDEO);
}
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_startReceiving(
        JNIEnv * env, jobject obj) {
    mVideoReceiver->startReceiving();
}
JNIEXPORT void JNICALL Java_constantin_fpv_1vr_VideoPlayer_stopReceiving(
        JNIEnv * env, jobject obj) {
    mVideoReceiver->stopReceiving();
}

}
