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
    // returns index of first SOI marker, or -1 if none found
    static ssize_t findNextEOI(const std::vector<uint8_t>& buff){
        for(size_t i=0;i<buff.size()-1;i+=2){
            if(buff[i]==0xFF &&  buff[i+1]==0xD9){
                return i;
            }
        }
        return -1;
    }
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
public:
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
        if (file.eof()) {
            MLOGD<<"Done with file";
            return std::nullopt;
        }
        const size_t pos=remaining();
        auto jpegData=readUpTo2(2);
        if(jpegData.size()!=2 || !isSOI(jpegData.data())){
            MLOGE<<"Expect SOI at begin of file"<<jpegData.size();
            return std::nullopt;
        }
        while(true){
            if (file.eof()) {
                MLOGD<<"Done with file";
                return std::nullopt;
            }
            auto tmp=readUpTo2(1024);
            const size_t indexEOI=findNextEOI(tmp);
            if(indexEOI>0){
                tmp.resize(indexEOI);
                jpegData.insert(jpegData.end(),tmp.begin(),tmp.end());
                return jpegData;
            }
            jpegData.insert(jpegData.end(),tmp.begin(),tmp.end());
            MLOGD<<"looping";
        }
    }
    std::optional<std::vector<uint8_t>> getNextFrame2(){
        if (file.eof()) {
            MLOGD<<"Done with file";
            return std::nullopt;
        }
        if(remaining()<4){
            MLOGE<<" Something went wrong. .remaining()<4";
            return std::nullopt;
        }
        std::vector<uint8_t> cachedData;
        cachedData.reserve(MAX_JPEG_SIZE);
        if(readUpTo(cachedData,2)!=2){
            MLOGE<<"cannot read first 2 bytes";
            return std::nullopt;
        }
        // check if SOI (start of image)
        if(!(cachedData[0]==0xFF &&cachedData[1]==0xD8)){
            MLOGE<<" expect first bytes to be SOI";
            return std::nullopt;
        }
        MLOGD<<" found SOI";
        if (remaining()<2) {
            MLOGE<<"Cannot find EOI - not at least 2 bytes left";
            return std::nullopt;
        }
        while(true){
            const
            const size_t bytesRead=readUpTo(cachedData,1024);
        }


        const std::size_t cachedDataOffset=cachedData.size();
        cachedData.resize(cachedDataOffset+1024);
        file.read((char *)&cachedData[cachedDataOffset],1024);
        const std::streamsize readData2 = file.gcount();
        cachedData.resize(cachedDataOffset+readData2);
        for(size_t j=cachedDataOffset;j<cachedData.size()-1;j+=2){
            if(cachedData[j]==0xFF && cachedData[j+1]==0xD9){
                MLOGD<<"EOI found";
                const size_t endOfJPEG=j+2;
                cachedData.resize(endOfJPEG);
                file.seekg(0, std::ios::end);
                return cachedData;
            }
        }

        MLOGE<<" cannot find EOI in 1024*1024 bytes";
        return std::nullopt;
    }
};


#endif //FPV_VR_OS_FILEREADERMJPEG_HPP
