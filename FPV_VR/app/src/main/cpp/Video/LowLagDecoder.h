//
// Created by Constantin on 29.05.2017.
//

#ifndef LOWLAGDECODERNATIVE
#define LOWLAGDECODERNATIVE
#define TAG "LowLagDecoder"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define MS_TO_NS 1000000;

#include <android/native_window.h>
#include <media/NdkMediaCodec.h>
#include <android/log.h>
#include <jni.h>
#include <iostream>
#include <thread>
#include "../Helper/Time.h"

#include <atomic>

using namespace std;

struct Decoder{
    bool configured= false;
    bool SW= false;
    bool limitFps= false;
    AMediaCodec *codec=NULL;
    ANativeWindow* window=NULL;
    AMediaFormat *format=NULL;
};
struct DecodingInfo{
    int nNALUS,nNALUSFeeded,nDecodedFrames;
    cMicroseconds lastFrame;
    cMicroseconds frameTimeSum;
    cMicroseconds frameTimeC;
    double currentFPS;

    cMicroseconds parsingLSum;
    cMicroseconds parsingLC;
    cMicroseconds waitInputBSum;
    cMicroseconds waitInputBC;
    cMicroseconds decodingTSum;
    cMicroseconds decodingTC;
};

static const int NALU_MAXLEN=1024*1024;

class LowLagDecoder {
public:
    LowLagDecoder(bool limitFps, ANativeWindow* window);
    void registerOnDecoderRatioChangedCallback(std::function<void(int,int)> decoderRatioChangedC);
    void registerOnDecoderFpsChangedCallback(std::function<void(float)> decoderFpsChangedC);
    void parseForNALU(uint8_t data[NALU_MAXLEN],int data_length,cMicroseconds creationT);
    void interpretNALU(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length,cMicroseconds creationT);

    void requestShutdown();
    void waitForShutdownAndDelete();

    void shutdown();
    int mWidth,mHeight;
private:
    void configureStartDecoder(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length);
    void feedDecoder(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length,cMicroseconds creationT);
    void checkOutputLoop();
    void printAvgLog();
    thread* mCheckOutputThread;
    Decoder decoder;
    DecodingInfo decodingInfo;
    cMicroseconds lastFrameLimitFPS;
    uint8_t CSDO[NALU_MAXLEN],CSD1[NALU_MAXLEN];
    int CSD0Length,CSD1Length;
    std::atomic<bool> running;
    std::atomic<bool>  isShuttingDown;
    std::function<void(int,int)> onDecoderRatioChanged;
    std::function<void(float)> onDecoderFpsChanged;
    cMilliseconds lastLogMS;
    uint8_t nalu_data[NALU_MAXLEN];
    int nalu_data_position=4;
    int nalu_search_state=0;
};


#endif //LOWLAGDECODERNATIVE
