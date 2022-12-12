#ifndef _MODELSOCKET_H_
#define _MODELSOCKET_H_

#include "IModel.h"

class cData
{
public:
    char data[32]={};
};

struct sData
{
    char data[32]={};
};

class ModelSocket : IModel
{
public:
    int type;
    int sizeData = 32;
    std::string ipaddr;
    int port;
    WSADATA wsdata;
    SOCKET socket;
    SOCKADDR_IN sockaddr;
    int wsOK;
    int sockaddr_size;
    pthread_t th;
    char cstr[32] = {};
    //char *cstr;
    std::string str;
    std::stringstream sendStream;
    std::stringstream recvStream;
    std::string sendString = " ";
    std::string recvString = " ";
    sData sendSData;
    sData recvSData;
    cData sendCData;
    cData recvCData;
    std::vector<char> vBuffer();
    int state = 0;
public:
    ModelSocket(){}
    ~ModelSocket(){}
};
#endif