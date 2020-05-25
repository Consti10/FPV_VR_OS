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

class SimpleEncoder {
private:
    //std::thread* mDecoderThread;
    std::thread* mEncoderThread= nullptr;
    //void loopDecoder();
    void loopEncoder();
    AMediaCodec* mediaCodec;
public:
    void start();
    void addBufferData(const uint8_t* data,const size_t data_len);
    void stop();
    std::mutex inputBufferDataMutex;
    std::vector<uint8_t> inputBufferData;
};


#endif //LIVEVIDEO10MS_SIMPLEENCODER_H
