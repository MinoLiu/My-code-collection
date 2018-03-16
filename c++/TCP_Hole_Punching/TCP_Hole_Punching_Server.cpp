#define BUF_SIZE 64
#define PORT 5000
#include "stdio.h"
#include "stdlib.h"
#include "winsock2.h"
#include "ws2tcpip.h"

#pragma comment(lib, "ws2_32.lib")

int main(){
    WSADATA         wsd;
    SOCKET          sServer, sClient[2];
    SOCKADDR_IN     servAddr;
    SOCKADDR_IN     clientAddr[2];
    char            buf[BUF_SIZE];
    char            client_ip_str[2][INET_ADDRSTRLEN];
    int             client_port[2];
    char            endpoint_str[2][BUF_SIZE];
    int             retVal;

    // init socket dll
    if(WSAStartup(MAKEWORD(2,2), &wsd) != 0){
        printf("WSAStartup failed!\n");
        return 1;
    }

    // create socket
    sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sServer == INVALID_SOCKET){
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // server address
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sServer, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR){
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(sServer);
        WSACleanup();
        return 1;
    }

    printf("Start listening...\n");
    retVal = listen(sServer, 1);
    if(retVal == SOCKET_ERROR){
        printf("listen failed!\n");
        WSACleanup();
        return 1;
    }
    // receive data
    int nClientLen = sizeof(clientAddr[0]);
    ZeroMemory(buf, BUF_SIZE);
    printf("Accept client request and receive data...\n");
    int i = 0;
    while(i < 2){
        sClient[i] = accept(sServer, (SOCKADDR*)&clientAddr[i], &nClientLen);
        if(sClient[i] == INVALID_SOCKET){
            printf("accept() failed!\n");
        }
        ZeroMemory(buf, BUF_SIZE);
        ZeroMemory(endpoint_str[i], BUF_SIZE);
        if(recv(sClient[i], buf, BUF_SIZE, 0) == SOCKET_ERROR){
            printf("recv() failed: %d\n", WSAGetLastError());
        }else{
            inet_ntop(AF_INET, &clientAddr[i].sin_addr, client_ip_str[i], sizeof(client_ip_str[i]));
            client_port[i] = ntohs(clientAddr[i].sin_port);

            strcpy(endpoint_str[i], client_ip_str[i]);
            strcat(endpoint_str[i], ":");
            char p_str[BUF_SIZE];
            sprintf(p_str, "%d", client_port[i]);
            strcat(endpoint_str[i], p_str);
            printf("From client %d: %s\t%s\n", i, endpoint_str[i], buf);
            i++;
        }

        if(send(sClient[0], endpoint_str[1], BUF_SIZE, 0) == SOCKET_ERROR){
            printf("send() failed: %d\n", WSAGetLastError());
        }

        if(send(sClient[i], endpoint_str[0], BUF_SIZE, 0) == SOCKET_ERROR){
            printf("send() failed: %d\n", WSAGetLastError());
        }
        printf("Both sent!\n");
        system("pause");
        closesocket(sServer);
        closesocket(sClient[0]);
        closesocket(sClient[1]);
        WSACleanup();
        return 0;
    }
}