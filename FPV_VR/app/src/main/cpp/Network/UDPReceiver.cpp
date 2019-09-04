/*********************************
 * MODE_NON_BLOCKING: as of 02.12.2017, MODE_BLOCKING should be preferred over MODE_NON_BLOCKING
 * - uses 1 core fully
 * - no timeout
 * - buffsize 1024*1024
 * MODE_BLOCKING
 * - timeout of 500ms
 * - buffsize 1024
 */

#include <sys/resource.h>
#include <cstring>
#include "UDPReceiver.h"
#include "../Helper/Time.h"
#include "../Helper/CPUPriorities.h"

#define TAG "UDPReceiver"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
//#define PRINT_HEARTBEAT
//#define PRINT_RECEIVED_BYTES

UDPReceiver::UDPReceiver(int port,int mode,string name,int CPUPriority,int buffsize,std::function<void(uint8_t[],int)> callback){
    mPort=port;
    mMode=mode;
    mName=name;
    mBuffsize=buffsize;
    mCPUPriority=CPUPriority;
    onDataReceivedCallback=callback;
}

void UDPReceiver::startReceiving() {
    receiving=true;
    mUDPReceiverThread=new thread([this] { this->receiveFromUDPLoop(); }); //Lambda-expression ?
}

void UDPReceiver::stopReceiving() {
    receiving=false;
    shutdown(mSocket,SHUT_RD);
    mUDPReceiverThread->join();
    close(mSocket);
    delete(mUDPReceiverThread);
}

void UDPReceiver::receiveFromUDPLoop() {
    if ((mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        LOGV("Error creating socket");
        return;
    }
    setCPUPriority(mCPUPriority,mName);
    struct sockaddr_in myaddr;
    memset((uint8_t *) &myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(mPort);
    if (::bind(mSocket, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        LOGV("Error binding Port; %d", mPort);
        return;
    }
#ifdef PRINT_HEARTBEAT
    cMilliseconds heartbeatTS = getTimeMS();
#endif
    /*if (mMode == MODE_NON_BLOCKING) {
        while (receiving) {
#ifdef PRINT_HEARTBEAT
            if ((getTimeMS() - heartbeatTS) > 1000) {
                LOGV("MODE_LOWLATENCY | %s | Heartbeat receiving on port %d\n",mName.c_str(), mPort);
                heartbeatTS = getTimeMS();
            }
#endif
            ssize_t message_length = recvfrom(mSocket, mBuffer, (size_t)mBuffsize, MSG_DONTWAIT, 0, 0);
            if (message_length > 0) { //else -1 was returned;timeout/No data received
#ifdef PRINT_RECEIVED_BYTES
                LOGV("%s: received %d bytes\n", mName.c_str(),(int) message_length);
#endif
                onDataReceivedCallback(mBuffer, (int) message_length);
            }
        }
    } else */
    if(mMode==MODE_BLOCKING){
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec=500000; //500ms. Just for safety (shutdown() also interrupts the receive)
        if (setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
            LOGV("SetSocketTimeout error");
        }
        uint8_t buff[mBuffsize];
        while (receiving) {
#ifdef PRINT_HEARTBEAT
            if ((getTimeMS() - heartbeatTS) > 1000) {
                LOGV("MODE_PERFORMANCE | %s | Heartbeat receiving on port %d\n",mName.c_str(), mPort);
                heartbeatTS = getTimeMS();
            }
#endif
            ssize_t message_length = recvfrom(mSocket, buff, (size_t)mBuffsize, MSG_WAITALL, 0, 0);
            if ( message_length > 0) { //else -1 was returned;timeout/No data received
#ifdef PRINT_RECEIVED_BYTES
                LOGV("%s: received %d bytes\n", mName.c_str(),(int) message_length);
#endif
                onDataReceivedCallback(buff, (int) message_length);
            }
        }
    }else{
        LOGV("UDPReceiver unknown mode");
    }
}

