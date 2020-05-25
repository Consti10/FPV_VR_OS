//
// Created by geier on 24/05/2020.
//

#include "SimpleEncoder.h"
#include <AndroidLogger.hpp>
#include "../MJPEGDecodeAndroid.hpp"
#include <chrono>
#include <asm/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <FileHelper.hpp>

void SimpleEncoder::start() {
    running=true;
    mEncoderThread=new std::thread(&SimpleEncoder::loopEncoder, this);
}

void SimpleEncoder::addBufferData(const uint8_t* data,const size_t data_len) {
    std::lock_guard<std::mutex> lock(inputBufferDataMutex);
    inputBufferData.resize(data_len);
    std::memcpy(inputBufferData.data(),data,data_len);
}

void SimpleEncoder::stop() {
    running=false;
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
    constexpr int COLOR_FormatYUV420Planar=19;
    constexpr int COLOR_FormatYUV422Flexible= 2135042184;
    constexpr int COLOR_FormatYUV422PackedPlanar=23;

    constexpr int COLOR_FormatYUV444Flexible=2135181448;

    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, COLOR_FormatYUV420Flexible);

    auto status=AMediaCodec_configure(mediaCodec,format, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    AMediaFormat_delete(format);
    if (AMEDIA_OK != status) {
        MLOGE<<"AMediaCodec_configure returned"<<status;
        return;
    }
    AMediaCodec_start(mediaCodec);
    int frameTimeUs=0;
    while(true){
        if(!running){
            break;
        }
        // Get input buffer if possible
        {
            std::lock_guard<std::mutex> lock(inputBufferDataMutex);
            if(!inputBufferData.empty()){
                const auto index=AMediaCodec_dequeueInputBuffer(mediaCodec,5*1000);
                if(index>0){
                    size_t inputBufferSize;
                    void* buf = AMediaCodec_getInputBuffer(mediaCodec,(size_t)index,&inputBufferSize);
                    MLOGD<<"Got input buffer "<<inputBufferSize;
                    mjpegDecodeAndroid.DecodeMJPEGtoEncoderBuffer(inputBufferData.data(),inputBufferData.size(),buf,640);
                    inputBufferData.resize(0);
                    //std::memset(buf,1,inputBufferSize);

                    AMediaCodec_queueInputBuffer(mediaCodec,index,0,inputBufferSize,frameTimeUs,0);
                    frameTimeUs+=16*1000;
                }
            }
        }
        {
            AMediaCodecBufferInfo info;
            const auto index=AMediaCodec_dequeueOutputBuffer(mediaCodec,&info,5*1000);
            AMediaCodec_getOutputFormat(mediaCodec);
            if(index==AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED){
                MLOGD<<"AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED";
                const std::string fn=FileHelper::findUnusedFilename(GROUND_RECORDING_DIRECTORY,"mp4");
                mFD = open(fn.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                mediaMuxer=AMediaMuxer_new(mFD,AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4);
                AMediaFormat* format=AMediaCodec_getOutputFormat(mediaCodec);
                videoTrackIndex=AMediaMuxer_addTrack(mediaMuxer,format);
                const auto status=AMediaMuxer_start(mediaMuxer);
                MLOGD<<"Media Muxer status "<<status;
                AMediaFormat_delete(format);
            }else if(index==AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED){
                MLOGD<<"AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED";
            }
            if(index>0){
                MLOGD<<"Got output buffer "<<index;
                size_t outputBufferSize;
                const auto* data = (const uint8_t*)AMediaCodec_getOutputBuffer(mediaCodec,(size_t)index,&outputBufferSize);

                AMediaMuxer_writeSampleData(mediaMuxer,videoTrackIndex,data,&info);

                AMediaCodec_releaseOutputBuffer(mediaCodec,index,false);
            }
        }
        MLOGD<<"Hi from worker";
    }
    if(mediaMuxer!=nullptr){
        AMediaMuxer_stop(mediaMuxer);
        AMediaMuxer_delete(mediaMuxer);
        close(mFD);
    }
    AMediaCodec_stop(mediaCodec);

    AMediaCodec_delete(mediaCodec);
}

