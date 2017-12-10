#include <iostream>
using namespace std;
#include <winsock2.h>
#include <stdlib.h>
#pragma comment(lib, "ws2_32.lib")

#define CLIENT_EXIT_OK      0
#define CLIENT_DLL_ERROR    1
#define CLIENT_API_ERROR    2
#define MAX_NUM_BUF         64
#define SERVERPORT          5555

char bufRecv[MAX_NUM_BUF];
char bufSend[MAX_NUM_BUF];
SOCKET sHost;
bool bConning;

void InitMember(void);
int ExitClient(int nExit);
bool RecvLine(SOCKET s, char* buf);
void ShowErrorMsg(void);

int main(){
    InitMember();

    WSADATA wsaData;
    int retVal;
    retVal = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(0 !=retVal){
        MessageBox(NULL, "Can not find a usable Windows Sockets dll!" , "ERROR", MB_OK);
        return CLIENT_DLL_ERROR;
    }

    sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(INVALID_SOCKET == sHost){
        ShowErrorMsg();
        WSACleanup();
        return CLIENT_API_ERROR;
    }

    cout << "Client succeeded!" << endl;
    cout << "Be ready to connect to server..." << endl;

    LPHOSTENT hostEntry;
    char hostname[MAX_NUM_BUF];
    gethostname(hostname, MAX_NUM_BUF);
    hostEntry = gethostbyname(hostname);
    if(!hostEntry){
        ShowErrorMsg();
        return ExitClient(CLIENT_API_ERROR);
    }
    SOCKADDR_IN addrServ;
    addrServ.sin_family = AF_INET;
    addrServ.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
    addrServ.sin_port = htons(SERVERPORT);

    retVal = connect(sHost, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));

    if(SOCKET_ERROR == retVal){
        ShowErrorMsg();
        return ExitClient(CLIENT_API_ERROR);
    }else{
        bConning = TRUE;
    }
    cout << "Connect successfully!" << endl;
    strcpy(bufSend, "Hello, Server!\n");
    retVal = send(sHost, bufSend, strlen(bufSend), 0);
    if(SOCKET_ERROR == retVal){
        ShowErrorMsg();
        return ExitClient(CLIENT_API_ERROR);
    }
    if(!RecvLine(sHost, bufRecv)){
        ShowErrorMsg();
        return ExitClient(CLIENT_API_ERROR);
    }
    cout << bufRecv << endl;
    system("pause");
    return ExitClient(CLIENT_EXIT_OK);
}

void ShowErrorMsg(void){
    int nErrCode = WSAGetLastError();
    HLOCAL hlocal = NULL;

    bool fOK = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, nErrCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (PTSTR)&hlocal, 0, NULL);
    if(hlocal != NULL){
        MessageBox(NULL, (char*)LocalLock(hlocal), "CLIENT ERROR", MB_OK);
        LocalFree(hlocal);
    }
}

void InitMember(void){
    memset(bufRecv, 0, MAX_NUM_BUF);
    memset(bufSend, 0, MAX_NUM_BUF);

    sHost = INVALID_SOCKET;

    bConning = FALSE;
}

int ExitClient(int nExit){
    closesocket(sHost);
    WSACleanup();
    cout << "Client exiting..." << endl;
    Sleep(20000);
    return nExit;
}

bool RecvLine(SOCKET s, char* buf){
    bool retVal = TRUE;
    bool bLineEnd = FALSE;
    int nReadLen = 0;
    int nDataLen = 0;
    while(!bLineEnd && bConning){
        nReadLen = recv(s, buf+nDataLen, 1,0);

        if(SOCKET_ERROR == nReadLen){
            retVal = FALSE;
            break;
        }

        if( 0 == nReadLen){
            retVal = FALSE;
            break;
        }

        if('\n' == *(buf+nDataLen)){
            bLineEnd = TRUE;
        }else {
            nDataLen += nReadLen;
        }
    }
    return retVal;
}