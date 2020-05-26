//
// Created by geier on 24/05/2020.
//

#ifndef LIVEVIDEO10MS_SIMPLEENCODER_H
#define LIVEVIDEO10MS_SIMPLEENCODER_H

#include <android/native_window.h>
#include <media/NdkMediaCodec.h>
#include <android/log.h>
#include <jni.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <media/NdkMediaMuxer.h>
#include <MJPEGDecodeAndroid.hpp>

class SimpleEncoder {
private:
    //std::thread* mDecoderThread;
    std::thread* mEncoderThread= nullptr;
    std::atomic<bool> running;
    //void loopDecoder();
    void loopEncoder();
    AMediaCodec* mediaCodec;
    const std::string GROUND_RECORDING_DIRECTORY;
    int mFD;
    MJPEGDecodeAndroid mjpegDecodeAndroid;
public:
    SimpleEncoder(const std::string GROUND_RECORDING_DIRECTORY1):GROUND_RECORDING_DIRECTORY(GROUND_RECORDING_DIRECTORY1){
    }
    void start();
    void addBufferData(const uint8_t* data,const size_t data_len);
    void stop();
    std::mutex inputBufferDataMutex;
    std::vector<uint8_t> inputBufferData;
    size_t videoTrackIndex;
    AMediaMuxer* mediaMuxer=nullptr;
};


#endif //LIVEVIDEO10MS_SIMPLEENCODER_H
