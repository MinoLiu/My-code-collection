//nonblocking_Client.cpp
/*  伺服器程式共有三個:
    nonblocking_Server.cpp
    nonblocking_Server(Client_Function).cpp
    nonblocking_Server(Client_Class).h

*/

#include <cstring>
#include <iostream>
#include <winsock2.h> //winsock2.h include <windows.h>, windows.h include <winbase.h>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

const int CLIENT_SETUP_FAIL = 1;        //啟動用戶端失敗
const int CLIENT_CREATETHREAD_FAIL = 2; //創建執行緒失敗
const int CLIENT_CONNECT_FAIL = 3;      //網路連結斷開
const int TIMEFOR_THREAD_EXIT = 1000;   //子執行緒退出時間
const int TIMEFOR_THREAD_SLEEP = 500;   //執行緒睡眠時間

const char* SERVERIP = "127.0.0.1";     //伺服器IP
const int SERVERPORT = 5566;            //伺服器TCP port
const int MAX_NUM_BUF = 48;             //緩衝區的最大長度
const char ADD = '+';
const char SUB = '-';
const char MUT = '*';
const char DIV = '/';
const char EQU = '=';

//資料包類型
const char EXPRESSION = 'E';        //算數運算式
const char BYEBYE = 'B';            //消息ByeBye

//資料包頭結構 該結構在WIN32XP下為4Bytes
typedef struct Head
{
    char type;      //類型
    u_short len;    //資料包的長度 (包括頭長度)
}hdr, *phdr;
const int HEADERLEN = sizeof(hdr);   //頭長度

//資料包中的資料結構
typedef struct Data
{
    char buf[MAX_NUM_BUF];
}DATABUF, *pDataBuf;

SOCKET client;              //客戶端Socket
HANDLE threadSend;          //傳送資料執行緒控制碼
HANDLE threadRecv;          //接收資料執行緒控制碼
DATABUF bufSend;            //傳送資料緩衝區
DATABUF bufRecv;            //接收資料緩衝區
CRITICAL_SECTION csSend;    //臨界區物件，確保主執行緒和傳送資料執行緒對BufSend的互斥存取
CRITICAL_SECTION csRecv;    //臨界區物件，確保主執行緒和傳送資料執行緒對BufRecv的互斥存取
bool isSendData;            //通知傳送資料執行緒傳送資料
bool isConnecting;          //與伺服器的連線狀態
HANDLE eventShowDataResult; //顯示計算結果的事件
HANDLE threads[2];          //子執行緒陣列


bool initClient();              //初始化
bool connectServer();           //連接伺服器
bool createSendAndRecvThread(); //創建發送和接收資料的執行緒
void inputAndOutput();          //使用者輸入資料
void exitClient();              //退出

void initMember();      //初始化全域變數
bool initSocket();      //初始化Socket

DWORD __stdcall recvDataThread(void* param);  //接收資料執行緒
DWORD __stdcall sendDataThread(void* param);  //傳送資料執行緒

bool packByeBye(const char* expr);      //將輸入的("byebye")字串打包
bool packExpression(const char* expr);  //將輸入的算數運算式打包

void showConnectMsg(bool surc);     //顯示連接伺服器的消息
void showDataResultMsg();              //顯示計算結果
void showTipMsg(bool firstInput);   //顯示提示資訊


int main()
{
    //初始化
    if(!initClient())
    {
        exitClient();
        return CLIENT_SETUP_FAIL;
    }

    //連接伺服器
    if(connectServer())
    {
        showConnectMsg(true);
    }else{
        showConnectMsg(false);
        exitClient();
        return CLIENT_SETUP_FAIL;
    }

    //創建發送和接收資料執行緒
    if(!createSendAndRecvThread())
    {
        exitClient();
        return CLIENT_CREATETHREAD_FAIL;
    }

    //使用者輸入資料和顯示結果
    inputAndOutput();

    //退出
    exitClient();system("pause");

    return 0;
}

/**
 * 初始化
 */
bool initClient()
{
    //初始化全域變數
    initMember();

    //創建SOCKET
    if(!initSocket())
    {
        return false;
    }

    return true;
}


/**
 * 初始化全域變數
 */
void initMember()
{
    //初始化臨界區
    InitializeCriticalSection(&csSend);
    InitializeCriticalSection(&csRecv);

    client = INVALID_SOCKET;  //通訊端
    threadRecv = nullptr;     //接收資料執行緒控制碼
    threadSend = nullptr;     //發送資料執行緒控制碼
    isConnecting = false;     //為連接狀態
    isSendData = false;       //不發送資料狀態

    //初始化資料緩衝區
    memset(bufSend.buf, 0, MAX_NUM_BUF);
    memset(bufRecv.buf, 0, MAX_NUM_BUF);
    memset(threads, 0, 2);

    //手動設置事件，初始化為無信號狀態
    eventShowDataResult = (HANDLE)CreateEvent(nullptr,true,false,nullptr);
}

/**
 * 創建非阻塞通訊端
 */

bool initSocket()
{
    int retValue;   //返回值
    WSADATA wsData; //WSADATA變數
    retValue = WSAStartup(MAKEWORD(2,2), &wsData);//初始化windows socket dll

    //創建通訊端
    client = socket(AF_INET, SOCK_STREAM, 0);
    if(client == INVALID_SOCKET)
        return false;


    //設置通訊端非阻塞模式
    u_long ul = 1;
    retValue = ioctlsocket(client, FIONBIO, (u_long*)&ul);
    if(retValue == SOCKET_ERROR)
        return false;

    return true;
}

/**
 * 連接伺服器
 */
bool connectServer()
{
    int retValue;           //返回值
    sockaddr_in serverAddr; //伺服器位置

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVERPORT);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);

    cout<< "linking...\n [exit:ctrl+C]";  //中文不能打 我用UTF-8
    while(true)
    {
        //連接伺服器
        retValue = connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr));

        //處理連接錯誤
        if(retValue == SOCKET_ERROR)
        {
            int errorCode = WSAGetLastError();
            switch(errorCode){
                case WSAEWOULDBLOCK:
                case WSAEINVAL:
                case WSAEALREADY:
                    continue;//連接未完成

                case WSAEISCONN:
                    goto exit;//連接已完成

                default:
                    return false;//連接失敗
            }

        }else

        if(retValue == 0)//連接成功
            break;
    }
exit:
    isConnecting = true;
    return true;
}

/**
 * 顯示連接伺服器失敗資訊
 */
void showConnectMsg(bool surc)
{
    if(surc)
    {
         cout<<"******************************"<<endl;
         cout<<"*                            *"<<endl;
         cout<<"* Succeed to connect server! *"<<endl;
         cout<<"*                            *"<<endl;
         cout<<"******************************"<<endl;
    }else{
         cout<<"***************************"<<endl;
         cout<<"*                         *"<<endl;
         cout<<"* Fail to connect server! *"<<endl;
         cout<<"*                         *"<<endl;
         cout<<"***************************"<<endl;
    }


}

/**
 * 創建發送和接收資料執行緒
 */
bool createSendAndRecvThread()
{
    //創建接收資料執行緒
    u_long threadID;
    threadRecv = CreateThread(nullptr, 0, recvDataThread, nullptr, 0, &threadID);
    if(threadRecv == nullptr)
        return false;

    //創建發送資料的執行緒
    threadSend = CreateThread(nullptr, 0, sendDataThread, nullptr, 0, &threadID);
    if(threadSend == nullptr)
        return false;

    //添加到執行緒陣列
    threads[0] = threadRecv;
    threads[1] = threadSend;
    return true;
}

/**
 * 輸入資料和顯示結果
 */
void inputAndOutput()
{
    char input[MAX_NUM_BUF];    //用戶輸入緩衝區
    bool isFirstInput = true;   //第一次只能輸入算術運算式

    while(isConnecting)
    {
        memset(input, 0, MAX_NUM_BUF);

        showTipMsg(isFirstInput);   //提示輸入資訊

        cin.getline(input, MAX_NUM_BUF);//輸入運算式

        char* temp = input;
        if(isFirstInput)                //第一次輸入
        {
            if(!packExpression(temp))   //算術運算式
            {
                continue;               //重新輸入
            }
            isFirstInput = false;       //成功輸入第一個算術運算式

        }else if(!packByeBye(temp)) //"byebye"打包
        {
            if(!packExpression(temp))   //算術運算式打包
            {
                continue;           //重新輸入
            }
        }

        //等待顯示計算結果
        if(WAIT_OBJECT_0 == WaitForSingleObject(eventShowDataResult, INFINITE))
        {
            ResetEvent(eventShowDataResult);    //設置為無信號狀態
            if(!isConnecting)                   //用戶端被動退出，此時接收和發送執行緒已經退出
            {
                break;
            }

            showDataResultMsg();    //顯示資料

            if(!strcmp(bufRecv.buf, "OK"))   //用戶端主動退出
            {
                isConnecting = false;
                Sleep(TIMEFOR_THREAD_EXIT); //給資料接收和發送執行緒退出時間
            }
        }
    }

    if(!isConnecting)   //與伺服器連結已經斷開
    {
        showConnectMsg(false);//顯示資訊
    }

    //等待資料發送和接受執行緒
    DWORD retValue = WaitForMultipleObjects(2, threads, true, INFINITE);
    if(retValue == WAIT_ABANDONED_0)
    {
        int errorCode = GetLastError();//不知道要幹嘛[標記]
    }

}

/**
 * 打包計算運算式的資料
 */
bool packExpression(const char* expr)
{

    char* temp = (char*)expr;

    while(*temp == ' ') //while(!*temp)
        ++temp;

    char* pos1 = temp;  //第一個數字位置
    char* pos2 = NULL;  //運算元位子
    char* pos3 = NULL;  //第二個數字位置
    int len1 = 0;       //第一個數字長度
    int len2 = 0;       //運算子長度
    int len3 = 0;       //第二個數字長度

    //第一個字元必須是+ - 或數字
    if(*temp != '+' &&
       *temp != '-' &&
       *temp < '0' || *temp > '9')
    {
        return false;
    }


    if(*temp++ == '+' && (*temp < '0' || *temp > '9' )) //第一個字元是'+'時，第二個字元必須是數字
        return false;   //重新輸入
    --temp;             //上移指標


    if(*temp++ == '-' && (*temp < '0' || *temp > '9' )) //第一個字元是'-'時，第二個字元必須是數字
        return false;   //重新輸入
    --temp;             //上移指標

    char* num = temp;   //數字起始位址
    if(*temp == '+' || *temp == '-')    // + - 號
        ++temp;

    while(*temp >= '0' && *temp <= '9') //數字
        ++temp;

    len1 = temp - num;  //數字長度

    //可能有空格
    while(*temp == ' ') //while(! *temp)
        ++temp;

    //算術運算子
    if(*temp != ADD &&
       *temp != SUB &&
       *temp != MUT &&
       *temp != DIV )
        return false;

    pos2 = temp;
    len2 = 1;

    //下移指標
    ++temp;
    //可能有空格
    while(*temp == ' ') //while(! *temp)
        ++temp;

    //第2個數字位址
    pos3 = temp;
    if(*temp < '0' || *temp > '9')
        return false;   //重新輸入

    while(*temp >= '0' && *temp <= '9') //數字
        ++temp;

    while(*temp == ' ') //while(! *temp)
        ++temp;

    if(*temp != EQU)    //最後是等號
        return false;   //重新輸入

    len3 = temp - pos3; //數字長度

    int exprLen = len1 + len2 + len3;   //算術表示長度

    //運算式讀入發送資料緩衝區
    EnterCriticalSection(&csSend);      //進入臨界區
    //資料包頭
    phdr header = (phdr)(bufSend.buf);
    header->type = EXPRESSION;          //類型
    header->len = exprLen + HEADERLEN;  //資料包長度
    //拷貝資料
    memcpy(bufSend.buf + HEADERLEN, pos1, len1);
    memcpy(bufSend.buf + HEADERLEN + len1, pos2, len2);
    memcpy(bufSend.buf + HEADERLEN + len1 + len2, pos3, len3);
    LeaveCriticalSection(&csSend);      //離開臨界區
    header = nullptr;

    isSendData = true;          //通知發資料執行緒發送資料

    return true;
}

/**
 * 打包發送byebye資料
 */
bool packByeBye(const char* expr)
{
    bool retValue = false;
    //  strcmp(): true=0 false=1
    if(!strcmp("ByeBye", expr) || !strcmp("byebye", expr))  //如果是"ByeBye"或"byebye"
    {
        EnterCriticalSection(&csSend);                      //進入臨界區
        phdr header = (phdr)(bufSend.buf);                  //強制轉換
        header->type = BYEBYE;                              //類型
        header->len = HEADERLEN + strlen("ByeBye");         //資料包長度
        memcpy(bufSend.buf + HEADERLEN, expr, strlen(expr));//複製資料
        LeaveCriticalSection(&csSend);                      //離開臨界區

        header = nullptr;
        isSendData = true;                          //通知發送資料執行緒
        retValue = true;
    }

    return retValue;
}

/**
 * 運算式結果
 */
void showDataResultMsg()
{
    EnterCriticalSection(&csRecv);
    cout<<"******************************"<<endl;
    cout<<"*                            *"<<endl;
    cout<<"* Result :                   *"<<endl;
    cout<<  bufRecv.buf  <<endl;
    cout<<"*                            *"<<endl;
    cout<<"******************************"<<endl;
    LeaveCriticalSection(&csRecv);

}

/**
 * 提示資訊
 */
 void showTipMsg(bool isFirstInput)
 {
     if(isFirstInput)
     {
        cout<<"*********************************"<<endl;
        cout<<"*                               *"<<endl;
        cout<<"* Please input expression.      *"<<endl;
        cout<<"* Usage: NumberOperatorNumber=  *"<<endl;
        cout<<"*                               *"<<endl;
        cout<<"*********************************"<<endl;
     }else{
        cout<<"*********************************"<<endl;
        cout<<"*                               *"<<endl;
        cout<<"* Please input expression.      *"<<endl;
        cout<<"* Usage: NumberOperatorNumber=  *"<<endl;
        cout<<"*                               *"<<endl;
        cout<<"* If you want to exit.          *"<<endl;
        cout<<"* Usage: ByeBye or byebye       *"<<endl;
        cout<<"*                               *"<<endl;
        cout<<"*********************************"<<endl;
     }

 }

 /**
  * 用戶端退出
  */
void exitClient()
{
    DeleteCriticalSection(&csSend);
    DeleteCriticalSection(&csRecv);
    CloseHandle(threadSend);
    CloseHandle(threadRecv);
    closesocket(client);
    WSACleanup();
    return;
}

/**
 * 發送資料執行緒
 */
DWORD __stdcall sendDataThread(void* param)
{
    while(isConnecting)                     //連接狀態
    {
        if(isSendData)                      //發送資料
        {
            EnterCriticalSection(&csSend);  //進入臨界區
            while(true)
            {
                int bufLen = ((phdr)(bufSend.buf))->len;
                int val = send(client, bufSend.buf, bufLen, 0);

                //處理返回錯誤
                if(val == SOCKET_ERROR)
                {
                    int errCode = WSAGetLastError();
                    if(errCode == WSAEWOULDBLOCK)       //發送緩衝區不可用
                    {
                        continue;                       //繼續迴圈
                    }else{
                        LeaveCriticalSection(&csSend);  //離開臨界區
                        isConnecting = false;           //斷開狀態
                        SetEvent(eventShowDataResult);  //通知主執行緒，防止再無限期地等待返回結果
                        return 0;
                    }
                }

                isSendData = false; //發送狀態
                break;              //跳出迴圈
            }
            LeaveCriticalSection(&csSend);  //離開臨界區
        }

        Sleep(TIMEFOR_THREAD_SLEEP);        //執行緒睡眠
    }

    return 0;
}

/**
 * 接收資料執行緒
 */
DWORD __stdcall recvDataThread(void* param)
{
    int retValue;
    char temp[MAX_NUM_BUF];
    memset(temp, 0, MAX_NUM_BUF);

    while(isConnecting) //連接狀態
    {
        //接收資料
        retValue = recv(client, temp, MAX_NUM_BUF, 0);

        if(retValue == SOCKET_ERROR)
        {
            int errCode = WSAGetLastError();
            if(errCode == WSAEWOULDBLOCK)   //資料緩衝區不可用
            {
                Sleep(TIMEFOR_THREAD_SLEEP);//等待
                continue;                   //繼續接收資料
            }else{
                isConnecting = false;
                SetEvent(eventShowDataResult);  //通知主執行緒，防止再無限期地等待
                return 0;                       //執行緒退出
            }
        }

        if(retValue == 0)       //伺服器關閉
        {
            isConnecting = false;           //執行緒退出
            SetEvent(eventShowDataResult);  //通知主執行緒，防止無限期等待
            return 0;                       //執行緒退出
        }

        if(retValue > HEADERLEN && retValue != -1)//收到資料
        {
            //對資料解包
            phdr header = (phdr)temp;
            EnterCriticalSection(&csRecv);
            memset(bufRecv.buf, 0, MAX_NUM_BUF);
            memcpy(bufRecv.buf, temp+HEADERLEN, header->len - HEADERLEN);   //將資料結果複製到接收資料緩衝區
            LeaveCriticalSection(&csRecv);

            SetEvent(eventShowDataResult);      //通知主執行緒顯示計算結果
            memset(temp, 0, MAX_NUM_BUF);
        }

        Sleep(TIMEFOR_THREAD_SLEEP);            //等待
    }
    return 0;
}