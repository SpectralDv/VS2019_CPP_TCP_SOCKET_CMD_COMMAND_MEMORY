


typedef struct{
int x;
int y;
int z;
}StructData;

StructData sendData;

void fillSendData()
{
    sendData.x=10;
    sendData.y=20;
    sendData.z=30;
}

/*
////////////////////////////////////
typedef struct 
{
    WORD _1:1;
    WORD _2:1;
    WORD _3:1;
    WORD _4:1;
    WORD _5:1;
}REG1;
typedef struct 
{
    WORD _1:1;
    WORD _2:1;
    WORD _3:1;
}REG2;
*/