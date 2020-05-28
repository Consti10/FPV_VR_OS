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
public:
    void open(const std::string FILENAME){
        file=std::ifstream(FILENAME.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            MLOGE<<"Cannot open file "<<FILENAME;
            return;
        }
        file.seekg(0, std::ios::end);
        FILE_SIZE=file.tellg();
        file.seekg(0, std::ios::beg);
        MLOGD << "File size is " << FILE_SIZE;
    }
    void close(){
        file.close();
    }
    // returns index of first SOI marker, or -1 if none found
    /*ssize_t findNextSOI(){
        for(size_t i=0;i<cachedData.size()-1;i+=2){
            if(cachedData[i]==0xFF && cachedData[i+1]==0xD8){
                return i;
            }
        }
        return -1;
    }
    ssize_t findNextEOI(){
        for(size_t i=0;i<cachedData.size()-1;i+=2){
            if(cachedData[i]==0xFF && cachedData[i+1]==0xD9){
                return i;
            }
        }
        return -1;
    }*/
    std::size_t remaining(){
        const auto currentPosition=file.tellg();
        return FILE_SIZE - currentPosition;
    }
    std::optional<std::vector<uint8_t>> getNextFrame(){
        if (file.eof()) {
            MLOGD<<"Done with file";
            return std::nullopt;
        }
        if(remaining()<4){
            MLOGE<<" Something went wrong. .tellg()<4";
            return std::nullopt;
        }
        std::vector<uint8_t> cachedData;
        cachedData.reserve(MAX_JPEG_SIZE);
        cachedData.resize(2);
        file.read((char *)cachedData.data(),cachedData.size());
        const std::streamsize readData = file.gcount();
        if(readData != 2){
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
        for(int i=0;i<1024;i++){
            if(file.eof()){
                MLOGE<<" No data available while searching for EOI";
                return std::nullopt;
            }
            const std::size_t cachedDataOffset=cachedData.size();
            cachedData.resize(cachedDataOffset+1024);
            file.read((char *)&cachedData[cachedDataOffset],1024);
            const std::streamsize readData2 = file.gcount();
            cachedData.resize(cachedDataOffset+readData2);
            for(size_t j=cachedDataOffset;j<cachedData.size()-1;j+=2){
                if(cachedData[j]==0xFF && cachedData[j+1]==0xD9){
                    MLOGD<<"EOI found";
                    cachedData.resize(j+2);
                    file.seekg(0, std::ios::end);
                    return cachedData;
                }
            }
        }
        MLOGE<<" cannot find EOI in 1024*1024 bytes";
        return std::nullopt;
    }
};


#endif //FPV_VR_OS_FILEREADERMJPEG_HPP
