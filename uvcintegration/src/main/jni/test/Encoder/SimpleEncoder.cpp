//
// Created by geier on 24/05/2020.
//

#include "SimpleEncoder.h"
#include <AndroidLogger.hpp>
#include <chrono>
#include <asm/fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <FileHelper.hpp>
#include "MediaCodecInfo.hpp"

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
    //mediaCodec= AMediaCodec_createCodecByName("OMX.google.h264.encoder");

    const int32_t WIDTH = 640;
    const int32_t HEIGHT = 480;
    const int32_t FRAME_RATE = 30;
    const int32_t BIT_RATE= 5*1024*1024;

    constexpr size_t HALF_WIDTH= WIDTH / 2;
    constexpr size_t HALF_HEIGHT= HEIGHT / 2;

    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, HEIGHT);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, WIDTH);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, BIT_RATE);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, FRAME_RATE);
    AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_I_FRAME_INTERVAL,30);

    AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_STRIDE,640);
    //AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_STRIDE,640);
    //AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_COLOR_FORMAT,)
    // Taken from https://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities#COLOR_Format24bitRGB888
    using namespace MediaCodecInfo::CodecCapabilities;
    constexpr auto ENCODER_COLOR_FORMAT=COLOR_FormatYUV420SemiPlanar;


    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT,ENCODER_COLOR_FORMAT);

    auto status=AMediaCodec_configure(mediaCodec,format, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);

    MLOGD<<"Media format:"<<AMediaFormat_toString(format);

    AMediaFormat_delete(format);
    if (AMEDIA_OK != status) {
        MLOGE<<"AMediaCodec_configure returned"<<status;
        return;
    }
    AMediaCodec_start(mediaCodec);
    int frameTimeUs=0;
    int frameIndex=0;
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
                    uint8_t* buf = AMediaCodec_getInputBuffer(mediaCodec,(size_t)index,&inputBufferSize);
                    MLOGD<<"Got input buffer "<<inputBufferSize;
                    //mjpegDecodeAndroid.DecodeMJPEGtoEncoderBuffer(inputBufferData.data(),inputBufferData.size(),buf,640);
                    MyColorSpaces::YUV422Planar<WIDTH,HEIGHT> bufferYUV422{};
                    mjpegDecodeAndroid.decodeToYUV422(bufferYUV422, inputBufferData.data(),
                                                      inputBufferData.size());

                    auto& framebuffer= *static_cast<MyColorSpaces::YUV420SemiPlanar<640,480>*>(static_cast<void*>(buf));
                    // copy Y component (easy)
                    memcpy(framebuffer.planeY, bufferYUV422.planeY, sizeof(bufferYUV422.planeY));

                    // copy CbCr component ( loop needed)
                    for(int i=0;i<WIDTH/2;i++){
                        for(int j=0;j<HEIGHT/2;j++){
                            auto tmp=bufferYUV422.getUVHalf(i,j);
                            framebuffer.setUV(i,j,tmp[0],tmp[1]);
                        }
                    }
                    inputBufferData.resize(0);
                    //std::memset(buf,1,inputBufferSize);
                    AMediaCodec_queueInputBuffer(mediaCodec,index,0,inputBufferSize,frameTimeUs,0);
                    frameTimeUs+=8*1000;
                }
            }
            /*const auto index=AMediaCodec_dequeueInputBuffer(mediaCodec,5*1000);
            if(index>0){
                size_t inputBufferSize;
                void* buf = AMediaCodec_getInputBuffer(mediaCodec,(size_t)index,&inputBufferSize);
                MLOGD<<"Got input buffer "<<inputBufferSize;
                YUVFrameGenerator::generateFrame(frameIndex,ENCODER_COLOR_FORMAT,(uint8_t*)buf,inputBufferSize);
                frameIndex++;

                AMediaCodec_queueInputBuffer(mediaCodec,index,0,inputBufferSize,frameTimeUs,0);
                frameTimeUs+=8*1000;
            }*/
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
                MLOGD<<"Output format:"<<AMediaFormat_toString(format);
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

