
#include "pack_data.h"
#include "ControllerSocket.h"
#include "ControllerData.h"
#include "ControllerInput.h"

//g++ -o main.exe main.cpp -lws2_32 -lpthread

class ViewMain
{
private:
    ControllerSocket* controllerSocket;
    ControllerData* controllerData;
    ControllerInput* controllerInput;
public:
    ViewMain(int type)
    {
        controllerSocket = new ControllerSocket(type);
        controllerData = new ControllerData();
        controllerInput = new ControllerInput();
        UpdateControllerSocket((IController*)controllerData);
        UpdateControllerInput((IController*)controllerData);
        UpdateControllerInput((IController*)controllerSocket);

        controllerSocket->InitSocket(IP_ADDR,PORT);
    }
    ~ViewMain(){delete controllerSocket;}
    void UpdateControllerSocket(IController* controller)
	{
		this->controllerSocket->AddObserver((IObserver*)controller);
	}
    void UpdateControllerInput(IController* controller)
	{
		this->controllerInput->AddObserver((IObserver*)controller);
	}
    void Update()
    {
        controllerSocket->UpdateSocket();
        do
        {
            if(controllerInput->Input()==1){break;}
        }
        while (GetKeyState(VK_ESCAPE) >= 0);
    }
};

///*
int main()
{
    ViewMain *viewMain;

    char str;
    printf("s - server\nc - client\n");
    scanf("%c",&str);

    //и клиент и сервер работает с одним сокетом
    if(str=='s')
    {
        viewMain = new ViewMain(NET_SERVER);
    }
    if(str=='c')
    {
        viewMain = new ViewMain(NET_CLIENT);
    }

    viewMain->Update();
    delete viewMain;
    return 0;
}
//*/

