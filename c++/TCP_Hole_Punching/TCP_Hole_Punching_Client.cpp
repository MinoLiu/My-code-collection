#define BUF_SIZE 64
#define PORT    5000
#define IP "0.0.0.0"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")

int main(){
    WSADATA     wsd;
    SOCKET      s, s_HolePunching_listen, s_HolePunching_connect, sClient;
    SOCKADDR_IN servAddr, localAddr, peerAddr, clientAddr, test;
    bool        flag = TRUE;
    char        buf_send[BUF_SIZE], buf_rcv1[BUF_SIZE], buf_rcv2[BUF_SIZE];
    int         peerPort, retVal;
    int         sLen = sizeof(SOCKADDR_IN);
    unsigned long ul = 1;

    // init socket dll
    if(WSAStartup(MAKEWORD(2,2), &wsd) != 0){
        printf("WSAStartup failed!\n");
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( s == INVALID_SOCKET){
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // setting SO_REUSEADDR option
    if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(BOOL)) == -1){
        printf("setsockopt on s: error\n");
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP);
    servAddr.sin_port = htons(PORT);

    printf("connect to Rendezvous Server...\n");
    retVal = connect(s, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if(retVal == SOCKET_ERROR){
        printf("connect() failed!\n");
        WSACleanup();
        return 1;
    }

    LPHOSTENT hostEntry;
    char hostname[BUF_SIZE];
    gethostname(hostname, BUF_SIZE);
    hostEntry = gethostbyname(hostname);
    struct in_addr ** addr_list;
    addr_list = (struct in_addr **)hostEntry->h_addr_list; // localhost's ip list

    struct sockaddr_in local;
    memset(&local, 0, sLen);
    getsockname(s, (struct sockaddr*)&local, &sLen);
    int local_port = ntohs(local.sin_port);
    ZeroMemory(buf_send, BUF_SIZE);
    strcpy(buf_send, "(Registration message)");
    if(send(s, buf_send, BUF_SIZE, 0) == SOCKET_ERROR){
        printf("send() failed: %d\n", WSAGetLastError());
    }
    printf("Already sent Registration message!\n");

    char str_local_endpoint[BUF_SIZE];
    memset(&str_local_endpoint, 0, BUF_SIZE);
    strcpy(str_local_endpoint, inet_ntoa(*addr_list[0]));
    strcat(str_local_endpoint, ":");
    memset(&buf_rcv1, 0, BUF_SIZE);
    sprintf(buf_rcv1, "%d", local_port);
    strcat(str_local_endpoint, buf_rcv1);
    printf("waiting peer's endpoint...\n");
    ZeroMemory(buf_rcv1, BUF_SIZE);
    if(recv(s, buf_rcv1, BUF_SIZE, 0) == SOCKET_ERROR){
        printf("rcv() failed: %d\n", WSAGetLastError());
    }
    memset(&buf_rcv2, 0, BUF_SIZE);
    strcpy(buf_rcv2, buf_rcv1);
    strtok(buf_rcv1, ":");
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_addr.s_addr = inet_addr(buf_rcv1);
    int ip_len = strlen(buf_rcv1);
    memset(&buf_rcv1, 0, BUF_SIZE);
    strcpy(buf_rcv1, buf_rcv2+ip_len+1);
    sscanf(buf_rcv1, "%d", &peerPort);
    peerAddr.sin_port = htons(peerPort);
    printf("\t-->%s\n", buf_rcv2);

    s_HolePunching_connect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s_HolePunching_connect == INVALID_SOCKET){
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    if(setsockopt(s_HolePunching_connect, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(BOOL)) == -1){
        printf("setsockopt on s_HolePunching_connect: error\n");
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*addr_list[0]));
    localAddr.sin_port = htons(local_port);

    if(bind(s_HolePunching_connect, (SOCKADDR*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR){
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(s_HolePunching_connect);
        WSACleanup();
        return 1;
    }

    s_HolePunching_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s_HolePunching_listen == INVALID_SOCKET){
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    if(setsockopt(s_HolePunching_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(BOOL)) == -1){
        printf("setsockopt on s_HolePunching_listen: error\n");
    }

    if(bind(s_HolePunching_listen, (SOCKADDR*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR){
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(s_HolePunching_listen);
        WSACleanup();
        return 1;
    }
    // non-blocking
    retVal = ioctlsocket(s_HolePunching_listen, FIONBIO, (unsigned long*)&ul);
    retVal == listen(s_HolePunching_listen, 1);
    if(retVal == SOCKET_ERROR){
        printf("listen failed!\n");
        WSACleanup();
        return 1;
    }

    bool c = false, a = false;
    while(c == false && a ==false){
        if(c == false){
            retVal = connect(s_HolePunching_connect, (SOCKADDR*)&peerAddr, sizeof(peerAddr));
            if(retVal == SOCKET_ERROR){
                retVal = WSAGetLastError();
                printf("connect() failed: %d\n", retVal);
            }else{
                c= true;
            }
        }
        if(a == false){
            sClient = accept(s_HolePunching_listen, (SOCKADDR*)&clientAddr, &sLen);
            if(sClient == INVALID_SOCKET){
                printf("accept() failed: %d\n", WSAGetLastError());
            }else{
                a = true;
            }
        }
    }
    printf("TCP NAT Punching succeeds..\n");

    if(a == true){
        ZeroMemory(buf_send, BUF_SIZE);
        strcpy(buf_send, "Hello, this is your peer at ");
        strcat(buf_send, str_local_endpoint);
        strcat(buf_send, "\n");
        if(send(sClient, buf_send, BUF_SIZE, 0) == SOCKET_ERROR)
            printf("send() failed: %d\n", WSAGetLastError());
        ZeroMemory(buf_rcv1, BUF_SIZE);
        if(recv(sClient, buf_rcv1, BUF_SIZE, 0) == SOCKET_ERROR)
            printf("recv() failed: %d\n", WSAGetLastError());
        else
            printf("%s\n", buf_rcv1);
    }
    if(c == true){
        ZeroMemory(buf_send, BUF_SIZE);
        strcpy(buf_send, "Hello, this is your peer at ");
        strcat(buf_send, str_local_endpoint);
        strcat(buf_send, "\n");
        if(send(s_HolePunching_connect, buf_send, BUF_SIZE, 0) == SOCKET_ERROR)
            printf("send() failed: %d\n", WSAGetLastError());
        ZeroMemory(buf_rcv1, BUF_SIZE);
        if(recv(s_HolePunching_connect, buf_rcv1, BUF_SIZE, 0) == SOCKET_ERROR)
            printf("recv() failed: %d\n", WSAGetLastError());
        else
            printf("%s\n", buf_rcv1);
    }

    system("pause");
    closesocket(s);
    WSACleanup();
    return 0;
}