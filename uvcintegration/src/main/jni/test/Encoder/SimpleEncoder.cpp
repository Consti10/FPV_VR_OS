//
// Created by geier on 24/05/2020.
//

#include "SimpleEncoder.h"
#include <AndroidLogger.hpp>

void SimpleEncoder::start() {
    mEncoderThread=new std::thread(&SimpleEncoder::loopEncoder, this);
}

void SimpleEncoder::addBufferData(const uint8_t* data,const size_t data_len) {
    std::lock_guard<std::mutex> lock(inputBufferDataMutex);
    inputBufferData.resize(data_len);
    std::memcpy(inputBufferData.data(),data,data_len);
}

void SimpleEncoder::stop() {
    if(mEncoderThread->joinable()){
        mEncoderThread->join();
    }
    delete(mEncoderThread);
    mEncoderThread= nullptr;
}


//void SimpleEncoder::loopDecoder() {}

void SimpleEncoder::loopEncoder() {
    AMediaFormat* format = AMediaFormat_new();
    mediaCodec = AMediaCodec_createEncoderByType("video/avc");

    uint32_t flags = AMEDIACODEC_CONFIGURE_FLAG_ENCODE;

    const int32_t width = 640;
    const int32_t height = 480;
    const int32_t frameRate = 30;
    const int32_t bitRate = 1024*1024;

    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, bitRate);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, frameRate);
    AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_I_FRAME_INTERVAL,30);
    //AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_COLOR_FORMAT,)
    // Taken from https://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities#COLOR_Format24bitRGB888
    constexpr int COLOR_Format24bitRGB888=12;
    constexpr int COLOR_FormatYUV420Flexible=2135033992;
    constexpr int COLOR_FormatYUV420SemiPlanar=21;

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, COLOR_FormatYUV420SemiPlanar);

    auto status=AMediaCodec_configure(mediaCodec,format, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    AMediaFormat_delete(format);
    if (AMEDIA_OK != status) {
        MLOGE<<"AMediaCodec_configure returned"<<status;
        return;
    }
    AMediaCodec_start(mediaCodec);
    AMediaCodecBufferInfo info;
    for(int i=0;i<10;i++){
        // Get input buffer if possible
        {
            const auto index=AMediaCodec_dequeueInputBuffer(mediaCodec,5*1000);
            if(index>0){
                size_t inputBufferSize;
                void* buf = AMediaCodec_getInputBuffer(mediaCodec,(size_t)index,&inputBufferSize);
                MLOGD<<"Got input buffer "<<inputBufferSize;
            }
        }
        {
            const auto index=AMediaCodec_dequeueOutputBuffer(mediaCodec,&info,5*1000);
            if(index>0){
                MLOGD<<"Got output buffer";
                size_t outputBufferSize;
                void* buf = AMediaCodec_getOutputBuffer(mediaCodec,(size_t)index,&outputBufferSize);
            }
        }
        MLOGD<<"Hi from worker";
    }
    AMediaCodec_stop(mediaCodec);

    AMediaCodec_delete(mediaCodec);
}

