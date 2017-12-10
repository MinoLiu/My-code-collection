#include <iostream>
using namespace std;

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 5556 
#define MAX_NUM_BUF 48
BOOL InitSocket();
void ManageSocket(SOCKET);
SOCKET sServer; // 監聽客戶端連線socket


int main(int argc, char* argv[])
{
    if(!InitSocket()){
        closesocket(sServer);
        WSACleanup();
        return 1;
    }
    cout << "Manage Socket...\n";
    ManageSocket(sServer);
    WSACleanup();
    return 0;
}

BOOL InitSocket(){
    int reVal;
    // 初始化 windows Sockets DLL
    WSADATA wsData;
    reVal = WSAStartup(MAKEWORD(2,2), &wsData);

    // 創建通訊端
    sServer = socket(AF_INET, SOCK_STREAM, 0);

    if(INVALID_SOCKET == sServer){
        return FALSE;
    }
    
    // 設置通訊端非阻塞模式
    unsigned long ul = 1;
    reVal = ioctlsocket(sServer, FIONBIO, (unsigned long*)&ul);
    if(SOCKET_ERROR == reVal){
        return FALSE;
    }

    // 綁定通訊端
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(SERVERPORT);
    serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    reVal = bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr));
    if(SOCKET_ERROR == reVal){
        return FALSE;
    }

    // 監聽
    reVal = listen(sServer, SOMAXCONN);
    if(SOCKET_ERROR == reVal){
        return FALSE;
    }
    return TRUE;
};

void ManageSocket(SOCKET listenSocket){
    SOCKET acceptSocket; // 接受客戶端連線請求Socket
    FD_SET socketSet;   // 伺服器Socket 集合
    FD_SET writeSet;    // 可寫 Socket 集合
    FD_SET readSet;     // 可讀 Socket 集合
    struct timeval tv; 
    char temp[MAX_NUM_BUF]; // 區域變數

    FD_ZERO(&socketSet);    //清空server socket
    FD_SET(listenSocket, &socketSet); // 加入監聽Socket
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while(TRUE){
        FD_ZERO(&readSet); // 清空可寫Socket集合
        FD_ZERO(&writeSet); // 清空可讀Socket集合
        readSet = socketSet;
        writeSet = socketSet; // 賦值

        // 呼叫檢查socket 狀態
        int num = select(0, &readSet, &writeSet, NULL, &tv);
        if(num == SOCKET_ERROR) // select function呼叫失敗
        {
            printf("select() returned with error %d\n", WSAGetLastError());
            return;
        }

        // 檢查是否存在客戶端的連線請求
        if(FD_ISSET(listenSocket, &readSet)){
            // 接受客戶端請求
            acceptSocket = accept(listenSocket, NULL, NULL);
            if(INVALID_SOCKET != acceptSocket){
                FD_SET(acceptSocket, &socketSet); // 將該socket加入伺服器socket 集合
                FD_CLR(listenSocket, &readSet);
            }else {
                printf("accept() failed with error %d\n", WSAGetLastError());
                return;
            }
        }

        // 走訪所有的socket
        // u_int i 可從 1 開始, 因為socketSet.fd_array[0] 是 listenSocket.
        for( u_int i = 0; i < socketSet.fd_count; i++){
            SOCKET sAccept = socketSet.fd_array[i];
            if(FD_ISSET(sAccept, &readSet)){
                int reVal = recv(sAccept, temp, MAX_NUM_BUF, 0 );
                if(reVal > 0){
                    printf("\t>>> %s\n", temp);
                }
                if(reVal == 0 ) // Connection has been closed/reset/terminated.
                {
                    FD_CLR(sAccept, &socketSet);
                }
            }
            if(FD_ISSET(sAccept, &writeSet)) // 該socket可寫
            {
                i = i; // 呼叫輸出函式, 傳送資料
            }
        }
    }
}