#ifndef _CONTROLLERSOCKET_H_
#define _CONTROLLERSOCKET_H_

#include "ModelSocket.h"
#include "IController.h"
#include "IObserver.h"


class ControllerSocket : IController, IObservable
{
private:
    ModelSocket* mSocket;
    std::vector<ModelSocket*> vConnection;
    std::vector<IObserver*> observers;
    ModelString mString;
public:
    ControllerSocket(int typeSocket)
    {
        this->name="ControllerSocket";
        mSocket = new ModelSocket();
        mSocket->type=typeSocket;
    }
    ~ControllerSocket()
    {
        #if TARGET_PLATFORM == PLATFORM_WINDOWS
        if(mSocket->type==NET_CLIENT && mSocket->state==0)
        {
            mSocket->sendStream<<"disconnect";
            while(!(mSocket->sendStream.str().empty())){}
            //int res = shutdown(mSocket->socket,SD_SEND);
        }
        if(mSocket->type==NET_SERVER)
        {
            for (int i = 0; i < vConnection.size(); i++)
            {
                vConnection[i]->sendStream<<"disconnect";
                while(!(vConnection[i]->sendStream.str().empty())){}
                //closesocket(vConnection[i]->socket);
            }
        }
        WSACleanup();
        closesocket(mSocket->socket);
        printf("~ControllerSocket: \n");
        Sleep(1000);
        #endif
    }
    void AddObserver(IObserver* o)
	{
        if(DEBUG==1){printf("ControllerSocket add: %s\n",((IController*)o)->name.c_str());}
		observers.push_back(o);
	}
    void RemoveObserver(IObserver* o)
	{
		std::vector<IObserver*>::iterator itErase = observers.begin();
		for (itErase; itErase != observers.end(); itErase++)
		{
			if (*itErase == o) { observers.erase(itErase); break; }
		}
	}
    void Notify(ModelString mString)
	{
		for (auto o : this->observers)
		{
			((IController*)o)->UpdateController(mString);
		}
	}
    void UpdateController(ModelString mString) override
	{
        mSocket->str=mString.str;
        this->Input();
	}   
    int InitSocket(std::string ipAddr,int port)
    {
        mSocket->recvStream.str("");
        mSocket->sendStream.str("");
        mSocket->ipaddr = ipAddr;
        mSocket->port = port;
        #if TARGET_PLATFORM == PLATFORM_WINDOWS
		mSocket->wsOK = WSAStartup(MAKEWORD(2, 2), &mSocket->wsdata);
		if (mSocket->wsOK != 0) { std::cout << "Can't start winsock" << std::endl; }
        #endif
        
		// SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		mSocket->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		mSocket->sockaddr.sin_family = AF_INET; //AF_INET6
		mSocket->sockaddr.sin_port = htons(port);
		mSocket->sockaddr.sin_addr.S_un.S_addr = inet_addr(ipAddr.c_str());// ADDR_ANY;// INADDR_ANY;//
		mSocket->sockaddr_size = sizeof(mSocket->sockaddr);

        return 0;
    }
    void WaitConnect()
    {
        if(mSocket->type==NET_SERVER)
        {
            //биндит свой, слушает свой
            bind(mSocket->socket,(const SOCKADDR*)&(mSocket->sockaddr),sizeof(mSocket->sockaddr));

            listen(mSocket->socket,100);

            SOCKET connect_socket;
            SOCKADDR_IN connect_addr;
            int connect_addr_size = sizeof(connect_addr);

            //ожидание входящего подключения
            while(connect_socket = accept(mSocket->socket,(SOCKADDR*)&connect_addr,&connect_addr_size))
            {
                ModelSocket *connectSocket = new ModelSocket();
                connectSocket->socket=connect_socket;
                connectSocket->sockaddr=connect_addr;
                connectSocket->sockaddr_size=connect_addr_size;
                #if TARGET_PLATFORM == PLATFORM_WINDOWS
                connectSocket->wsOK = WSAStartup(MAKEWORD(2, 2), &connectSocket->wsdata);
                if (connectSocket->wsOK != 0) { std::cout << "Can't start winsock" << std::endl; }
                #endif
                #if TARGET_PLATFORM == PLATFORM_WINDOWS
                DWORD nonBlocking = 1;
                if(ioctlsocket(connectSocket->socket,FIONBIO,&nonBlocking)!=0)
                {
                    printf("failed to set non-blocking socket\n");
                    Sleep(2000);
                    //exit(0);
                }
                #endif
                vConnection.push_back(connectSocket);

				printf("%s", inet_ntoa(vConnection[vConnection.size()-1]->sockaddr.sin_addr));
				printf(": %d", ntohs(vConnection[vConnection.size()-1]->sockaddr.sin_port));
                printf(": connected\n");

                pthread_create(&(vConnection[vConnection.size()-1]->th),NULL,ControllerSocket::ThreadStub,(void*)this);
                //pthread_join(th, NULL);
		        pthread_detach(vConnection[vConnection.size()-1]->th);
            }
        }
        if(mSocket->type==NET_CLIENT)
        {
            //подключается к серверу
            int con = connect(mSocket->socket,(const SOCKADDR*)&(mSocket->sockaddr),sizeof(mSocket->sockaddr));
            if(con!=0){printf("Server disconnect\n");Sleep(1000);exit(0);}

            #if TARGET_PLATFORM == PLATFORM_WINDOWS
            DWORD nonBlocking = 1;
            if(ioctlsocket(mSocket->socket,FIONBIO,&nonBlocking)!=0)
            {
                printf("failed to set non-blocking socket\n");
                Sleep(2000);
                //exit(0);
            }
            #endif
            printf("%s", inet_ntoa(mSocket->sockaddr.sin_addr));
			printf(": %d", ntohs(mSocket->sockaddr.sin_port));
            printf(": connected \n");

            pthread_create(&(mSocket->th),NULL,ControllerSocket::ThreadStub,(void*)this);
            //pthread_join(th, NULL);
		    pthread_detach(mSocket->th);
        }
    }
    ///////////////////////////////////////////////
    virtual void ThreadProcWait()
   	{
        if(mSocket->type==NET_SERVER)
        {
            //ожадиет клиентов
		    this->WaitConnect();
        }
        if(mSocket->type==NET_CLIENT)
        {
            //работает с заданным сокетом
            //this->WaitConnect(mSocket);
        }
   	}
    static void *ThreadStubWait(void* self)
    {
		((ControllerSocket*)self)->ThreadProcWait();
		return 0;
    }
    ///////////////////////////////////////////////
    virtual void ThreadProc()
   	{
        if(mSocket->type==NET_SERVER)
        {
            //работает с подключившимся сокетом
		    this->ExchangeData(vConnection[vConnection.size()-1]);
        }
        if(mSocket->type==NET_CLIENT)
        {
            //работает с заданным сокетом
            this->ExchangeData(mSocket);
        }
   	}
    static void *ThreadStub(void* self)
    {
		((ControllerSocket*)self)->ThreadProc();
		return 0;
    }
    ///////////////////////////////////////////////
    void ExchangeData(ModelSocket *connectSocket)
    {
        int rb;
        int sb;
        
        //WSAEventSelect(connectSocket->socket,0,FD_READ);
        do
        {
            Sleep(10);
            //if(connectSocket->str!="")
            //if (mSocket->sendString != " ")
            //if(connectSocket->sendStream.str()!="")
            //{
                //sb = sendto(connectSocket->socket,mSocket->sendString.c_str(),strlen(mSocket->sendString.c_str()), 0, (SOCKADDR*)&(connectSocket->sockaddr), connectSocket->sockaddr_size);
                //sb = send(connectSocket->socket,mSocket->sendString.c_str(),strlen(mSocket->sendString.c_str()),0);
                //sb = send(connectSocket->socket,mSocket->sendStream.str().c_str(),mSocket->sendStream.str().length(),0);
                
                //printf("length: %d",connectSocket->str.length());
                //printf("  str: %s\n",connectSocket->str.c_str());
                //while(!(connectSocket->str.empty()))
            while(!(connectSocket->sendStream.str().empty()))
            {
                //std::cout<< " send: " <<connectSocket->sendStream.str();
                //sb = send(connectSocket->socket,connectSocket->str.c_str(),connectSocket->str.length(),0);
                sb = send(connectSocket->socket,connectSocket->sendStream.str().c_str(),connectSocket->sendStream.str().length(),0);
                //printf("  vstr: %s\n",connectSocket->str.c_str());
                //connectSocket->str = "";
                if(sb>0){
                connectSocket->sendStream.str("");}
                //printf(" :send success \n");
            }
            //}

            //rb = recvfrom(mSocket->socket, (char*)(mSocket->recvdata.c_str()), ,mSocket->sizeData, 0, (SOCKADDR*)&(connectSocket->sockaddr), &connectSocket->sockaddr_size);
            rb = recv(connectSocket->socket,(char*)(connectSocket->cstr),sizeof(connectSocket->cstr)-1,0);
            
            //rb = recv(mSocket->socket,(char*)(mSocket->recvString.c_str()),mSocket->sizeData,0);
            if(rb>=0)
            {
                connectSocket->cstr[rb]=0;
                if(strcmp(connectSocket->cstr,"disconnect") == 0)
                {
                    if(mSocket->type==NET_CLIENT)
                    {
                        mSocket->state=1;
                        //break;
                    }
                }
                //printf("rb: %d\n",rb);
                printf("%s", inet_ntoa(connectSocket->sockaddr.sin_addr));
				printf(": %d", ntohs(connectSocket->sockaddr.sin_port));
                
                printf(": %s\n",connectSocket->cstr);
                //connectSocket->recvStream<<connectSocket->cstr;
                connectSocket->recvString=connectSocket->cstr;
                //printf("%s\n",connectSocket->recvString.c_str());
                mString.str=connectSocket->recvString;
                Notify(mString);

                strcpy(connectSocket->cstr, "");
                connectSocket->recvStream.clear();
                connectSocket->recvString.clear();
            }  
        }
        while (GetKeyState(VK_ESCAPE) >= 0);
    }
    int Input()
    {
        //std::cin>>mSocket->str;
        //std::cin.getline((char*)(mSocket->str.c_str()), mSocket->sizeData);

        if(mSocket->type==NET_SERVER)
        {
            if(mSocket->str=="exit")
            {
                mSocket->str="disconnect";
                mSocket->state = 1;
                //while(!(mSocket->sendStream.str().empty())){}
            }
            else
            {
                for (int i = 0; i < vConnection.size(); i++)
                {
                    //mSocket->str+="/";
                    vConnection[i]->sendStream<<mSocket->str;
                    //printf("input: %s\n",mSocket->str.c_str());
                }
                mSocket->str = "";
                //while(!(mSocket->str.empty())){}
            }
        }
        if(mSocket->type==NET_CLIENT)
        {
            if(mSocket->str=="exit")
            {
                mSocket->sendStream<<"disconnect";
                while(!(mSocket->sendStream.str().empty())){}
                return 1;
            }
            else
            {
                //mSocket->str+="/";
                mSocket->sendStream<<mSocket->str;
                //printf("input: %s\n",mSocket->str.c_str());
                mSocket->str = "";
            }
        }
        return mSocket->state;
    }
    void UpdateSocket()
    {
        if(mSocket->type==NET_SERVER)
        {
            pthread_create(&(mSocket->th),NULL,ControllerSocket::ThreadStubWait,(void*)this);
            //pthread_join(th, NULL);
		    pthread_detach(mSocket->th);
        }
        if(mSocket->type==NET_CLIENT)
        {
            WaitConnect();
        }
        //Input();
    }
};
#endif