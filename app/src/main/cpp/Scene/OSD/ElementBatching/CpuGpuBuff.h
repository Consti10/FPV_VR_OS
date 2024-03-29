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

// Holds an dynamic memory array of fixed size
// Intention is to avoid common mistakes (e.g. modifying the array but forgetting to set _sizeModified )
// The intended usage of this class is a producer - consumer pattern:
// The producer changes the content of the memory area, after which this area is marked as "modified" (aka dirty)
// The consumer consumes this changed content, and after that it is no longer marked as "modified" (dirty)
template <class T>
class ModifiableArray{
private:
    //How many elements of member '_array' have been modified since last update call.
    unsigned int nModifiedElements=0;
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
        return nModifiedElements;
    }
    T* modify(unsigned int nElementsToModify){
        nModifiedElements=nElementsToModify;
        return _array.data();
    }
    T* modify(){
        return modify(size());
    }
    const T* update(){
        nModifiedElements=0;
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
    std::vector<std::shared_ptr<ModifiableArray<T>>> cpuBuffer={};
    // How many elements of type T this buffer holds (fragmented into chunks due to ModifiableArray)
    unsigned int nElements;
    // Total size of the buffer (same on GPU and CPU)
    unsigned int sizeBytes=0;
private:
    const std::string name;
public:
    explicit CpuGpuBuff(const std::string& name){
        glGenBuffers(1,&gpuBuffer);
    }
    std::shared_ptr<ModifiableArray<T>> allocate(const int n){
        auto data=std::make_shared<ModifiableArray<T>>(n);
        cpuBuffer.push_back(data);
        return data;
    }
    void setupGPUBuffer(){
        nElements=calculateCPUBufferSize();
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
