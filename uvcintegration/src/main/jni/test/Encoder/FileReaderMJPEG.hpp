//
// Created by geier on 27/05/2020.
//

#ifndef FPV_VR_OS_FILEREADERMJPEG_HPP
#define FPV_VR_OS_FILEREADERMJPEG_HPP

#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <sstream>
#include <fstream>
#include <AndroidLogger.hpp>
#include <android/asset_manager.h>
#include <optional>

class FileReaderMJPEG{
private:
    std::ifstream file;
    static constexpr size_t MAX_JPEG_SIZE=1024*1024;
    //std::array<uint8_t,MAX_JPEG_SIZE> cachedData;
    //size_t cachedDataSize=0;
    std::size_t FILE_SIZE;
    /*struct JPEGImage{
        std::size_t begin;
        std::size_t end;
    };
    std::vector<JPEGImage> images;*/
    // read up to maxSize bytes and returns the number of bytes read
    // increases size of buff by n of bytes read
    std::size_t readUpTo(std::vector<uint8_t>& buff,size_t maxWantedDataSize){
        const size_t offset=buff.size();
        buff.resize(offset + maxWantedDataSize);
        file.read((char*)&buff[offset], maxWantedDataSize);
        const std::streamsize readData = file.gcount();
        if(readData != maxWantedDataSize){
            MLOGD<<" couldnt read all data. Read "<<readData<<" Wanted "<<maxWantedDataSize;
        }
        buff.resize(offset+readData);
        return readData;
    }

public:
    std::vector<uint8_t> readUpTo2(size_t maxWantedDataSize){
        std::vector<uint8_t> ret(maxWantedDataSize);
        file.read((char*)ret.data(), maxWantedDataSize);
        const std::streamsize readData = file.gcount();
        ret.resize(readData);
        return ret;
    }
    std::size_t remaining(){
        const auto currentPosition=file.tellg();
        return FILE_SIZE - currentPosition;
    }
    static bool isSOI(const uint8_t* data){
        return data[0]==0xFF && data[1]==0xD8;
    }
    static bool isEOI(const uint8_t* data){
        return data[0]==0xFF && data[1]==0xD9;
    }
    // returns index of first SOI marker, or -1 if none found
    static ssize_t findNextEOI(const std::vector<uint8_t>& buff){
        for(size_t i=0;i<buff.size()-1;i++){
            if(buff[i]==0xFF &&  buff[i+1]==0xD9){
                return i;
            }
        }
        return -1;
    }
    void open(const std::string FILENAME){
        file=std::ifstream(FILENAME.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            MLOGE<<"Cannot open file "<<FILENAME;
            return;
        }
        file.seekg(0, std::ios::end);
        FILE_SIZE=file.tellg();
        if(FILE_SIZE<5){
            MLOGE<<"File too small";
        }
        MLOGD <<"File size is " << FILE_SIZE;
        file.seekg(0, std::ios::beg);
        const auto SOI=readUpTo2(2);
        if(!isSOI(SOI.data())){
            MLOGE<<" first 2 bytes not SOI";
        }
        file.seekg(-2, std::ios::end);
        const auto EOI=readUpTo2(2);
        if(!isEOI(EOI.data())){
            MLOGE<<" last 2 bytes not EOI";
        }
        file.seekg(0, std::ios::beg);
    }
    void close(){
        file.close();
    }
    std::optional<std::vector<uint8_t>> getNextFrame(){
        if (remaining()<4) {
            MLOGD<<"Done with file"<<remaining();
            return std::nullopt;
        }
        auto jpegData=readUpTo2(2);
        if(!isSOI(jpegData.data())){
            MLOGE<<"Expect SOI as first 2 bytes "<<jpegData.size();
            //MLOGD<<(int)jpegData[0]<<" "<<(int)jpegData[1];
            return std::nullopt;
        }
        bool lastByteWasMarker=0;
        while(true){
            if (remaining()==0) {
                MLOGE<<"couldn't find corresponding EOI";
                return std::nullopt;
            }
            const auto readBuffer=readUpTo2(1024*1024);
            MLOGD << " read " << readBuffer.size();
            // parse read data until EOI
            for(size_t i=0; i < readBuffer.size(); i++){
                jpegData.push_back(readBuffer[i]);
                if(lastByteWasMarker && readBuffer[i] == 0xD9){
                    const size_t wastedBytes= readBuffer.size()-i-1;
                    if(wastedBytes>0){
                        file.seekg(-wastedBytes, std::ios::cur);
                    }
                    MLOGD<<" Is SOI EOI "<<isSOI(jpegData.data())<<" "<<isEOI(&jpegData[jpegData.size()-2])<<" wasted bytes "<<wastedBytes<<" x "<<readBuffer.size();
                    MLOGD<<" next are "<<(int)readBuffer[i+1]<<" "<<(int)readBuffer[i+2];
                    return jpegData;
                }
                lastByteWasMarker = readBuffer[i] == 0xFF;
            }
        }
    }
};


#endif //FPV_VR_OS_FILEREADERMJPEG_HPP
