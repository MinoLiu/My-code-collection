#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* SERVERIP = "127.0.0.1";
const int SERVERPORT = 5556;
const int MAX_NUM_BUF = 48;

SOCKET client;

bool initSocket();
bool connectServer();

int main(){
	
	if(!initSocket()){
		closesocket(client);
		WSACleanup();
		return 1;
	}
	
	cout<<"trying connect to server...\n";
	
	if(connectServer()){
		cout<<"success connected... \n";
	}
	else{
		cout<<"connected failded...\n";
		closesocket(client);
		WSACleanup();
		return 1; 
	} 
	
	int retval;
	char temp[MAX_NUM_BUF];
	
	memset(temp, 0, MAX_NUM_BUF);
	cout<<"Enter messages send to server...\n";
	cin.getline(temp, MAX_NUM_BUF);
	retval = send(client, temp, MAX_NUM_BUF, 0);
	
	closesocket(client);
	WSACleanup();
		
	return 0;
}

bool initSocket(){
	int retVal;
	WSADATA wsData;
	retVal = WSAStartup(MAKEWORD(2,2), &wsData);
	
	client = socket(AF_INET, SOCK_STREAM, 0);
	
	if(client == INVALID_SOCKET)
		return false;
		
	return true;
}

bool connectServer(){
	int retVal;
	sockaddr_in servAddr;
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERVERPORT);
	servAddr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);
	
	cout<<"connecting...\n";
	while(true){
		retVal = connect(client, (sockaddr*)&servAddr, sizeof(servAddr));
		
		if(retVal != 0){
			cout<<"client : can't connect server \n";
			Sleep(2000);
		}
		
		if(retVal == 0)
			break;
		
	} 
	
	return true;
	
}
