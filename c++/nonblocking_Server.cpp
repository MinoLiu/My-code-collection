#include <iostream>
using namespace std;

#include <winsock2.h>
#include <list>
#include "nonblocking_Server(Client_class).h"

#pragma comment(lib, "ws2_32.lib")

typedef list<CClient*> CLIENTLIST;

#define SERVERPORT 5566
#define SERVER_SETUP_FAIL 1

#define TIMEFOR_THREAD_EXIT 5000
#define TIMEFOR_THREAD_HELP 1500
#define TIMEFOR_THREAD_SLEEP 500

HANDLE hThreadAccept;
HANDLE hThreadHelp;
SOCKET sServer;
BOOL bServerRuning;
HANDLE hServerEvent;
CLIENTLIST clientlist;
CRITICAL_SECTION csClientList;

BOOL InitServer(void);
BOOL StartService(void);
void StopService(void);
BOOL CreateHelpperAndAcceptThread(void);
void ExitServer(void);

void InitMember(void);
BOOL InitSocket(void);

void ShowTipMsg(BOOL bFirstInput);
void ShowServerStartMsg(BOOL bSuc);
void ShowServerExitMsg(void);

DWORD __stdcall HelperThread(void *pParam);
DWORD __stdcall AcceptThread(void *pParam);
void ShowConnectNum();

int main()
{    
    if(!InitServer())
    {        
        ExitServer();
        return SERVER_SETUP_FAIL;
    }
    if(!StartService())
    {
        ShowServerStartMsg(FALSE);
        ExitServer();
        return SERVER_SETUP_FAIL;
    }
    StopService();
    
    ExitServer();
    return 0;
}

BOOL InitServer(void)
{
    InitMember();
    if(!InitSocket())
    {
        return FALSE;
    }
    return TRUE;
}

void InitMember(void)
{
    InitializeCriticalSection(&csClientList);
    hServerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hThreadAccept = NULL;
    hThreadHelp = NULL;
    sServer = INVALID_SOCKET;
    bServerRuning = FALSE;
    clientlist.clear();
}

BOOL InitSocket(void)
{
    int reVal;
    WSADATA wsData;
    reVal = WSAStartup(MAKEWORD(2,2), &wsData);
    sServer = socket(AF_INET, SOCK_STREAM, 0);
    if(INVALID_SOCKET == sServer)
    {
        return FALSE;
    }
    unsigned long ul = 1;
    reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
    if(SOCKET_ERROR == reVal)
    {
        return FALSE;
    }
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SERVERPORT);
    serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
    if(SOCKET_ERROR == reVal){
        return FALSE;
    }    
    reVal = listen(sServer, SOMAXCONN);

    if(SOCKET_ERROR == reVal){        
        return FALSE;
    }
    return TRUE;
}

BOOL StartService(void)
{
    BOOL reVal = TRUE;
    ShowTipMsg(TRUE);

    char cInput;
    do
    {
        cin >> cInput;
        if('s' == cInput || 'S' == cInput)
        {
            if(CreateHelpperAndAcceptThread())
            {
                ShowServerStartMsg(TRUE);
            }else{
                reVal = FALSE;
            }
            break;
        }else{
            ShowTipMsg(TRUE);
        }
    } while(cInput != 's' && cInput != 'S');
    return reVal;
}

void StopService(void)
{
    BOOL reVal = TRUE;
    ShowTipMsg(FALSE);
    char cInput;
    for(;bServerRuning;){
        cin >> cInput;
        if(cInput == 'E' || cInput == 'e')
        {
            if(IDOK == MessageBox(NULL, "Are you sure?", "Server", MB_OKCANCEL)){
                break;
            }else {
                Sleep(TIMEFOR_THREAD_EXIT);
            }
        }else {
            Sleep(TIMEFOR_THREAD_EXIT);
        }
    }
    bServerRuning = FALSE;
    ShowServerExitMsg();
    Sleep(TIMEFOR_THREAD_EXIT);
    WaitForSingleObject(hServerEvent, INFINITE);
    return;
}

void ShowTipMsg(BOOL bFirstInput)
{
    if(bFirstInput)
    {
        cout << "\n\n";
        cout << "**************************\n";
        cout << "*                        *\n";
        cout << "*   s(S): Start server   *\n";
        cout << "*                        *\n";
        cout << "**************************\n";
        cout << "Please input: \n";
    } else {
        cout << "\n\n";
        cout << "**************************\n";
        cout << "*                        *\n";
        cout << "*   s(E): Exit server    *\n";
        cout << "*                        *\n";
        cout << "**************************\n";
        cout << "Please input: \n";
    }
}

void ExitServer(void)
{
    DeleteCriticalSection(&csClientList);
    CloseHandle(hServerEvent);
    closesocket(sServer);
    WSACleanup();
}

BOOL CreateHelpperAndAcceptThread(void)
{
    bServerRuning = TRUE;

    unsigned long ulThreadId;
    hThreadHelp = CreateThread(NULL, 0, HelperThread, NULL, 0, &ulThreadId);
    if(NULL == hThreadHelp)
    {
        bServerRuning = TRUE;
        return FALSE;
    }else {
        CloseHandle(hThreadHelp);
    }
    hThreadAccept = CreateThread(NULL, 0, AcceptThread, NULL, 0, &ulThreadId);
    if(NULL == hThreadAccept)
    {
        bServerRuning = FALSE;
        return FALSE;
    } else {
        CloseHandle(hThreadAccept);
    }
    return TRUE;
}

void ShowServerStartMsg(BOOL bSuc)
{
    if(bSuc)
    {
        cout << "**************************\n";
        cout << "*                        *\n";
        cout << "*   Server succeeded!    *\n";
        cout << "*                        *\n";
        cout << "**************************\n";
    }else {
        cout << "**************************\n";
        cout << "*                        *\n";
        cout << "*   Server failed   !    *\n";
        cout << "*                        *\n";
        cout << "**************************\n";
    }
}

void ShowServerExitMsg(void)
{
        cout << "**************************\n";
        cout << "*                        *\n";
        cout << "*   Server exit...       *\n";
        cout << "*                        *\n";
        cout << "**************************\n";
}

DWORD __stdcall AcceptThread(void* pParam)
{
    SOCKET sAccept;
    sockaddr_in addrClient;

    for(;bServerRuning;)
    {
        memset(&addrClient, 0, sizeof(sockaddr_in));
        int lenClient = sizeof(sockaddr_in);
        sAccept = accept(sServer, (sockaddr*)&addrClient, &lenClient);
        if(INVALID_SOCKET == sAccept)
        {
            int nErrCode = WSAGetLastError();
            if(nErrCode == WSAEWOULDBLOCK)
            {
                Sleep(TIMEFOR_THREAD_SLEEP);
                continue;
            }else {
                return 0;
            }
        }else {
            CClient *pClient = new CClient(sAccept, addrClient);
            EnterCriticalSection(&csClientList);
            clientlist.push_back(pClient);
            LeaveCriticalSection(&csClientList);
            pClient->StartRuning();
        }
    }
    return 0;
}

DWORD __stdcall HelperThread(void* pParam)
{
    for (;bServerRuning;)
    {
        EnterCriticalSection(&csClientList);
        CLIENTLIST::iterator iter = clientlist.begin();
        for(iter; iter != clientlist.end();)
        {
            CClient *pClient = (CClient*)*iter;
            if(pClient->IsExit())
            {
                clientlist.erase(iter++);
                delete pClient;
                pClient = NULL;
            }else {
                iter++;
            }
        }
        LeaveCriticalSection(&csClientList);
        Sleep(TIMEFOR_THREAD_HELP);
    }
    if(!bServerRuning)
    {
        EnterCriticalSection(&csClientList);
        CLIENTLIST::iterator iter = clientlist.begin();
        for(iter; iter !=clientlist.end();){
            CClient *pClient = (CClient*)*iter;
            if(pClient->IsConning())
            {
                pClient->DisConning();
            }
            ++iter;
        }
        LeaveCriticalSection(&csClientList);
        Sleep(TIMEFOR_THREAD_SLEEP);
        EnterCriticalSection(&csClientList);

        while( 0 != clientlist.size())
        {
            iter =clientlist.begin();
            for(iter; iter!=clientlist.end();)
            {
                CClient *pClient = (CClient*)*iter;
                if(pClient->IsExit())
                {
                    clientlist.erase(iter++);
                    delete pClient;
                    pClient = NULL;
                }else {
                    iter++;
                }
            }
            Sleep(TIMEFOR_THREAD_SLEEP);
        }
        LeaveCriticalSection(&csClientList);
    }
    clientlist.clear();
    SetEvent(hServerEvent);
    return 0;
}