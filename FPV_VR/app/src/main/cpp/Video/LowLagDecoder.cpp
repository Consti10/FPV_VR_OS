

#include <unistd.h>
#include <sstream>
#include <cstring>
#include "LowLagDecoder.h"
#include "../Helper/CPUPriorities.h"

//#define RELEASE  //Disables all debugging logs
#define BUFFER_TIMEOUT 20*1000 //20ms (a littlebit more than 16.6ms)
#define TIME_BETWEEN_LOGS 10*1000 //10s


LowLagDecoder::LowLagDecoder(bool limitFps, ANativeWindow* window){
    decoder.SW=false;
    decoder.limitFps=limitFps;
    //decoder.limitFps=false;

    decoder.window=window;
    decoder.configured=false;
    CSD0Length=0;
    CSD1Length=0;
    lastFrameLimitFPS=getSystemTimeUS();
    onDecoderRatioChanged=NULL;
    onDecoderFpsChanged=NULL;
    isShuttingDown=false;

    decodingInfo.nNALUS=0;
    decodingInfo.nNALUSFeeded=0;
    decodingInfo.nDecodedFrames=0;
    decodingInfo.lastFrame=0;
    decodingInfo.frameTimeSum=0;
    decodingInfo.frameTimeC=0;
    decodingInfo.currentFPS=0;
    decodingInfo.parsingLC=0;
    decodingInfo.parsingLSum=0;
    decodingInfo.waitInputBC=0;
    decodingInfo.waitInputBSum=0;
    decodingInfo.decodingTC=0;
    decodingInfo.decodingTSum=0;
    lastLogMS=getTimeMS();
    nalu_data_position=4;
    nalu_search_state=0;
}

void LowLagDecoder::registerOnDecoderRatioChangedCallback(std::function<void (int,int)> decoderRatioChangedC) {
    onDecoderRatioChanged=decoderRatioChangedC;
}

void LowLagDecoder::registerOnDecoderFpsChangedCallback(std::function<void (float)> decoderFpsChangedC) {
    onDecoderFpsChanged=decoderFpsChangedC;

}

void LowLagDecoder::parseForNALU(uint8_t *data, int data_length,cMicroseconds creationT) {
    for (int i = 0; i < data_length; ++i) {
        nalu_data[nalu_data_position++] = data[i];
        if (nalu_data_position == NALU_MAXLEN - 1) {
            nalu_data_position = 0;
        }
        switch (nalu_search_state) {
            case 0:
            case 1:
            case 2:
                if (data[i] == 0)
                    nalu_search_state++;
                else
                    nalu_search_state = 0;
                break;
            case 3:
                if (data[i] == 1) {
                    nalu_data[0] = 0;
                    nalu_data[1] = 0;
                    nalu_data[2] = 0;
                    nalu_data[3] = 1;
                    interpretNALU(nalu_data,nalu_data_position-4,getSystemTimeUS()-creationT);
                    nalu_data_position = 4;
                }
                nalu_search_state = 0;
                break;
            default:
                break;
        }
    }
}

void LowLagDecoder::interpretNALU(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length,cMicroseconds creationT){
    decodingInfo.nNALUS++;
    if(isShuttingDown){
        //A feedD thread (e.g. file or udp) thread might still be running
        //Don't feed a NALU to the decoder if release() has been called from another thread
        return;
    }
    if(decoder.configured){
        if(decoder.limitFps){
            cMicroseconds deltaSinceLastFrame=(cMicroseconds)(getSystemTimeUS()-lastFrameLimitFPS);
            int64_t waitTime=16000-deltaSinceLastFrame;
            if(waitTime>0){
                try{
                    usleep((useconds_t)waitTime);
                }catch (...){
                }
            }
            lastFrameLimitFPS=getSystemTimeUS();
            creationT=getSystemTimeUS();
        }
        feedDecoder(nalu_data,nalu_data_length,creationT);
        decodingInfo.nNALUSFeeded++;
    } else{
        configureStartDecoder(nalu_data,nalu_data_length);
    }
}

void LowLagDecoder::configureStartDecoder(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length){
    if(nalu_data_length<=4){
        //No data in NALU (e.g) at the beginning of a stream
        return;
    }
    int nal_unit_type = nalu_data[4]&0x1f;
    if(nal_unit_type==7){
        //LOGV("SPS");
        for(int i=0;i<nalu_data_length;i++){
            CSDO[i]=nalu_data[i];
            CSD0Length=nalu_data_length;
        }
    }else if(nal_unit_type==8){
        //LOGV("PPS");
        for(int i=0;i<nalu_data_length;i++){
            CSD1[i]=nalu_data[i];
            CSD1Length=nalu_data_length;
        }
    }else{
        //LOGV("No SPS/PPS. Length:%d",nalu_data_length);
    }
    if(CSD0Length==0||CSD1Length==0){
        //There is no CSD0/CSD1 data yet. We don't have enough information to initialize the decoder.
        return;
    }
    if(decoder.SW){
        decoder.codec = AMediaCodec_createDecoderByType("OMX.google.h264.decoder");
    }else {
        decoder.codec = AMediaCodec_createDecoderByType("video/avc");
    }
    decoder.format=AMediaFormat_new();
    AMediaFormat_setString(decoder.format,AMEDIAFORMAT_KEY_MIME,"video/avc");
    AMediaFormat_setInt32(decoder.format,AMEDIAFORMAT_KEY_WIDTH,1280);
    AMediaFormat_setInt32(decoder.format,AMEDIAFORMAT_KEY_HEIGHT,720);

    AMediaFormat_setBuffer(decoder.format,"csd-0",&CSDO,(size_t)CSD0Length);
    AMediaFormat_setBuffer(decoder.format,"csd-1",&CSD1,(size_t)CSD1Length);

    AMediaCodec_configure(decoder.codec, decoder.format, decoder.window, NULL, 0);
    if (decoder.codec==NULL) {
        LOGV("Dec. configure failed");
    }
    AMediaCodec_start(decoder.codec);
    decoder.configured=true;
    running=true;
    mCheckOutputThread=new thread([this] { this->checkOutputLoop(); }); //Lambda-expression ?
}
void LowLagDecoder::checkOutputLoop() {
    setCPUPriority(CPU_PRIORITY_DECODER_OUTPUT,"DecoderCheckOutput");
    AMediaCodecBufferInfo* info=new AMediaCodecBufferInfo;
    while(running) {
        ssize_t index=AMediaCodec_dequeueOutputBuffer(decoder.codec,info,BUFFER_TIMEOUT);
        if (index >= 0) {
            AMediaCodec_releaseOutputBufferAtTime(decoder.codec,(size_t)index,(int64_t)getSystemTimeNS());
            int64_t delta=getSystemTimeUS()-(cMicroseconds)info->presentationTimeUs;
            decodingInfo.decodingTSum+=delta;
            decodingInfo.decodingTC++;
            decodingInfo.nDecodedFrames++;
            cMicroseconds ts=getSystemTimeUS();
            if(decodingInfo.lastFrame!=0){
                cMicroseconds frameDelta=ts-decodingInfo.lastFrame;
                decodingInfo.frameTimeSum+=frameDelta;
                decodingInfo.frameTimeC++;
                const double fps=1.0/(decodingInfo.frameTimeSum/decodingInfo.frameTimeC/1000.0)*1000.0;
                if((int)decodingInfo.currentFPS!=(int)fps){
                    decodingInfo.currentFPS=fps;
                    if(onDecoderFpsChanged!=NULL){
                        onDecoderFpsChanged((float)decodingInfo.currentFPS);
                    }
                }
            }
            decodingInfo.lastFrame=getSystemTimeUS();

        } else if (index == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED ) {
            auto format = AMediaCodec_getOutputFormat(decoder.codec);
            AMediaFormat_getInt32(format,AMEDIAFORMAT_KEY_WIDTH,&mWidth);
            AMediaFormat_getInt32(format,AMEDIAFORMAT_KEY_HEIGHT,&mHeight);
            if(onDecoderRatioChanged!=NULL){
                onDecoderRatioChanged(mWidth,mHeight);
            }
            //LOGV("output format / buffers changed,%dx%d",mWidth,mHeight);
        } else if(index!=AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            LOGV("dequeueOutputBuffer not normal");
        }
    }
}

void LowLagDecoder::feedDecoder(uint8_t nalu_data[NALU_MAXLEN],int nalu_data_length,cMicroseconds creationT){
    cMicroseconds ts1=getSystemTimeUS();
    cMicroseconds deltaParsing=ts1-creationT;
    cMicroseconds tsBefore=ts1;
    while (running) {
        ssize_t index=AMediaCodec_dequeueInputBuffer(decoder.codec,BUFFER_TIMEOUT);
        size_t size;
        if (index >=0) {
            uint8_t* buf = AMediaCodec_getInputBuffer(decoder.codec,(size_t)index,&size);
            memcpy(buf, nalu_data,(size_t)nalu_data_length);
            cMicroseconds ts2=getSystemTimeUS();
            AMediaCodec_queueInputBuffer(decoder.codec, (size_t)index, 0, (size_t)nalu_data_length,ts2, 0);
            cMicroseconds deltaWaitIB=ts2-tsBefore;
            decodingInfo.waitInputBSum+=deltaWaitIB;
            decodingInfo.waitInputBC++;
            decodingInfo.parsingLSum+=deltaParsing;
            decodingInfo.parsingLC++;
            printAvgLog();
            return;
        }else if(index==AMEDIACODEC_INFO_TRY_AGAIN_LATER){
            //just try again
        }
    }
}

void LowLagDecoder::shutdown(){
    running=false;
    isShuttingDown=true;
    if(decoder.configured){
        if(mCheckOutputThread->joinable()){
            mCheckOutputThread->join();
        }
        delete(mCheckOutputThread);
        mCheckOutputThread=NULL;
        try{
            AMediaCodec_stop(decoder.codec);
            AMediaCodec_delete(decoder.codec);
            ANativeWindow_release(decoder.window);
        }catch(...){
            //Decoders can throw exceptions like crazy. To prevent an app crash
        }
    }
}

void LowLagDecoder::requestShutdown() {
    running=false;
    isShuttingDown=true;
}

void LowLagDecoder::waitForShutdownAndDelete() {
    if(decoder.configured){
        if(mCheckOutputThread->joinable()){
            mCheckOutputThread->join();
        }
        delete(mCheckOutputThread);
        mCheckOutputThread=NULL;
        try{
            AMediaCodec_stop(decoder.codec);
            AMediaCodec_delete(decoder.codec);
            ANativeWindow_release(decoder.window);
        }catch(...){
            //Decoders can throw exceptions like crazy. To prevent an app crash
        }
    }
}

void LowLagDecoder::printAvgLog() {
#ifndef RELEASE
    if(getTimeMS()-lastLogMS>TIME_BETWEEN_LOGS){
        lastLogMS=getTimeMS();
    }else{
        return;
    }
    std::ostringstream frameLog;
    double avgParsingT,avgWaitInputBT,avgDecodingT,avgDecodingLatencySum;
    if(decodingInfo.parsingLC>0){
        avgParsingT=decodingInfo.parsingLSum/decodingInfo.parsingLC;
    }else{
        avgParsingT=-1;
    }
    if(decodingInfo.waitInputBC>0){
        avgWaitInputBT=decodingInfo.waitInputBSum/decodingInfo.waitInputBC;
    }else{
        avgWaitInputBT=-1;
    }
    if(decodingInfo.decodingTC>0){
        avgDecodingT=decodingInfo.decodingTSum/decodingInfo.decodingTC;
    }else{
        avgDecodingT=-1;
    }
    if(avgParsingT>=0 && avgWaitInputBT>=0 && avgDecodingT>=0){
        avgDecodingLatencySum=avgParsingT+avgWaitInputBT+avgDecodingT;
    }else{
        avgDecodingLatencySum=-1;
    }
    avgParsingT/=1000.0;
    avgWaitInputBT/=1000.0;
    avgDecodingT/=1000.0;
    avgDecodingLatencySum/=1000.0;
    frameLog<<"......................Decoding Latency Averages......................"<<
            "\nParsing:"<<avgParsingT<<" | WaitInputBuffer:"<<avgWaitInputBT<<" | Decoding:"<<avgDecodingT<<" | Decoding Latency Sum:"<<avgDecodingLatencySum<<
            "\nN NALUS:"<<decodingInfo.nNALUS<<" | N NALUES feeded:"<<decodingInfo.nNALUSFeeded<<" | N Decoded Frames:"<<decodingInfo.nDecodedFrames<<
            "\nFPS:"<<decodingInfo.currentFPS;
    LOGV("%s",frameLog.str().c_str());
#endif
}


//----------------------------------------------------JAVA bindings---------------------------------------------------------------

/*#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_constantin_fpv_1vr_VideoPlayer_##method_name

inline jlong jptr(LowLagDecoder *lowLagDecoder) {
    return reinterpret_cast<intptr_t>(lowLagDecoder);
}

inline LowLagDecoder *native(jlong ptr) {
    return reinterpret_cast<LowLagDecoder *>(ptr);
}

extern "C" {

JNI_METHOD(jlong, nativeConstructLowLagDecoder)
(JNIEnv *env, jclass clazz,jboolean limitFPS,jobject surface) {
    return jptr(
            new LowLagDecoder(limitFPS,surface));
}

JNI_METHOD(void, nativeDestroyRenderer)
(JNIEnv *env, jclass clazz, jlong glRendererMono) {
    delete native(glRendererMono);
}

JNI_METHOD(void, nativeOnSurfaceCreated)
(JNIEnv *env, jobject obj, jlong glRendererMono,jobject assetManagerJAVA) {
    LOGV("OnSurfaceCreated()");
    native(glRendererMono)->OnSurfaceCreated(env,obj,assetManagerJAVA);
}

JNI_METHOD(void, nativeOnSurfaceChanged)
(JNIEnv *env, jobject obj, jlong glRendererMono,jint w,jint h) {
    LOGV("OnSurfaceChanged()");
    native(glRendererMono)->OnSurfaceChanged(w,h);
}

JNI_METHOD(void, nativeOnDrawFrame)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    //LOGV("OnDrawFrame()");
    native(glRendererMono)->OnDrawFrame();
}

JNI_METHOD(void, nativeOnPause)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    LOGV("OnPause()");
    native(glRendererMono)->OnPause();
}
JNI_METHOD(void, nativeOnResume)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    LOGV("OnResume()");
    native(glRendererMono)->OnResume();
}
/*JNI_METHOD(void, nativeOnSurfaceDestroyed)
(JNIEnv *env, jobject obj, jlong glRendererMono) {
    LOGV("OnResume()");
    native(glRendererMono)->OnSurfaceDestroyed();
}*/
//}
