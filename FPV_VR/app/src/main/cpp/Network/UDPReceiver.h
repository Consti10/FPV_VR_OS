
#ifndef UDPRECEIVER
#define UDPRECEIVER

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <android/log.h>
#include "pthread.h"
#include <iostream>
#include <thread>

using namespace std;

class UDPReceiver {
public:
    //UDPReceiver(int port,int buffsize,int timeoutMS,int mode);
    UDPReceiver(int port,int mode,string name,int CPUPriority,int buffsize,std::function<void(uint8_t[],int)>);
    void startReceiving();
    void stopReceiving();
    //Deprecated
    // static const int MODE_NON_BLOCKING=0; //Blocks 1 cpu core. Should be avoided (bad performance). Lowest latency however
    static const int MODE_BLOCKING=1; //blocks until 1024 bytes are received

    //static const int mBuffsize=1024;
private:
    void receiveFromUDPLoop();
    std::function<void(uint8_t[],int)> onDataReceivedCallback;
    int mPort;
    int mMode;
    bool receiving;
    string mName;
    int mSocket;
    int mCPUPriority;
    //uint8_t mBuffer[mBuffsize];
    thread* mUDPReceiverThread;
    int mBuffsize;
};

#endif