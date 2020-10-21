//
// Created by Constantin on 1/24/2019.
//

#ifndef FPV_VR_CPUGPUBUFF_H
#define FPV_VR_CPUGPUBUFF_H

#include <cstdio>
#include <GLES2/gl2.h>
#include <vector>
#include <string>
#include <array>
#include <android/log.h>


// Holds an array of fixed size
// Intention is to avoid common mistakes (e.g. modifying the array but forgetting to set _sizeModified )
template <class T> class ModifiableArray{
private:
    //How many elements of member '_array' have been modified since last update call.
    unsigned int _sizeModified=0;
    //The data this instance holds
    std::vector<T>_array;
public:
    explicit ModifiableArray(int size):
    _array{(unsigned)size}{
    };
    unsigned int size(){
        return (unsigned int)_array.size();
    }
    int sizeBytes(){
        return size()*sizeof(T);
    }
    unsigned int hasBeenModified()const{
        return _sizeModified;
    }
    T* modify(unsigned int sizeModified){
        _sizeModified=sizeModified;
        return _array.data();
    }
    T* modify(){
        return modify(size());
    }
    const T* update(){
        _sizeModified=0;
        return _array.data();
    }
    void zeroContent(){
        memset(modify(),0,(size_t)sizeBytes());
    }
};


template <class T>
class CpuGpuBuff{
public:
    GLuint gpuBuffer;
    std::vector<ModifiableArray<T>*> cpuBuffer={};
    //std::vector<Chunk<T>> chunks;
    unsigned int sizeBytes=0;
    unsigned int size;
private:
    const std::string name;
public:
    explicit CpuGpuBuff(const std::string& name){
        glGenBuffers(1,&gpuBuffer);
    }
    ModifiableArray<T>* allocate(const int n){
        auto data=new ModifiableArray<T>(n);
        cpuBuffer.push_back(data);
        return data;
    }
    void setupGPUBuffer(){
        size=calculateCPUBufferSize();
        sizeBytes= calculateCPUBufferSizeB();
        //LOGD("Size:%d nObj: %d",mSizeB,nObjects);
        glBindBuffer(GL_ARRAY_BUFFER,gpuBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeBytes,
                     nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //fill the buffer for the first time
        for(auto modArray : cpuBuffer){
            modArray->modify();
        }
        uploadToGpuIfModified();
    }
    void uploadToGpuIfModified(){
        int nOfUpdates=0;
        unsigned int nOfBytesUpdated=0;
        glBindBuffer(GL_ARRAY_BUFFER,gpuBuffer);
        int offsetB=0;
        //int bytesSaved=0;
        for(auto modArray : cpuBuffer){
            const auto sizeModified=modArray->hasBeenModified();
            if(sizeModified>0){
                const auto bytesToUpdate=sizeModified*sizeof(T);
                glBufferSubData(GL_ARRAY_BUFFER,offsetB,bytesToUpdate,modArray->update());
                nOfUpdates++;
                nOfBytesUpdated+=bytesToUpdate;
                //bytesSaved+=modArray->sizeBytes()-bytesToUpdate;
            }
            offsetB+=modArray->sizeBytes();
        }
        glBindBuffer(GL_ARRAY_BUFFER,0);
        // __android_log_print(ANDROID_LOG_DEBUG,"T","bytes saved %d %d",bytesSaved,nOfBytesUpdated);
        //LOGD("%s: N of glBufferSubData calls: %d | Kbyte: %f",name.c_str(),nOfUpdates,nOfBytesUpdated/1024.0);
    }
    unsigned int calculateCPUBufferSize(){
        unsigned int ret=0;
        for(const auto buff : cpuBuffer){
            ret+=buff->size();
        }
        return ret;
    }
    unsigned int calculateCPUBufferSizeB(){
        return calculateCPUBufferSize()* sizeof(T);
    }
};



/*class Chunk{
public:
    int offsetB=0;
    int lengthB=0;
    ModifiableArray* data;
};

using Index=uint32_t;

class TextBatcher{
public:
    GLuint gpuBuffer;
    explicit TextBatcher(){
        glGenBuffers(1,&gpuBuffer);
    }
    Index allocate(){
        std::vector<uint8_t> data=std::vector<uint8_t>();
        cpuBuffer.push_back(data);
        return (unsigned int)cpuBuffer.size();
    }
    std::vector<uint8_t>& modify(Index index){
        return cpuBuffer.at(index);
    }
    void update(){
        sizeBytes=0;
        for(const auto& buff:cpuBuffer){
            sizeBytes+=buff.size();
        }
        glBindBuffer(GL_ARRAY_BUFFER,gpuBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeBytes,
                     nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        //
        glBindBuffer(GL_ARRAY_BUFFER,gpuBuffer);
        unsigned int offsetB=0;
        for(const auto& buff:cpuBuffer){
            glBufferSubData(GL_ARRAY_BUFFER,offsetB,buff.size(),buff.data());
            offsetB+=buff.size();
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
private:
    std::vector<std::vector<uint8_t >> cpuBuffer;
    unsigned int sizeBytes=0;
};*/

#endif //FPV_VR_CPUGPUBUFF_H
