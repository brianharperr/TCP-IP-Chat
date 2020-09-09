/*
	Chat.c  - TCP/IP Chat application with Host/Client functionality.
	Author - Brian Harper
	Version 1.0
	Live TCP/IP Server+Client on port 8888
*/

#define _WIN32_WINNT 0x0501

#include<io.h>
#include<stdio.h>
#include<winsock2.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI receive(void* socket) {
	SOCKET* s = (SOCKET*)socket;
	char msg[255];
	while (1) {
		memset(msg, 0, sizeof(msg));
		if (recv(*s, msg, 255, 0) == SOCKET_ERROR) {
			printf("Connection Error, WSAError: %d\n", WSAGetLastError());
			break;
		}else {
			printf(">%s",msg);
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	char message[255];
	char ip[INET_ADDRSTRLEN] = { 0 };
	int c = sizeof(struct sockaddr_in);
	//
	DWORD thread_id;
	HANDLE thread;
	//
	printf("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised...");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created...");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	printf("\nHit enter to become a host, otherwise enter the host's IP.\n");
	fgets(ip, sizeof(ip), stdin);

	//Host Functionality
	if (ip[0] == '\n') {
		//Bind socket
		if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("Bind failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		printf("Bind done @ ");

		//inet_ntop(AF_INET,&(server.sin_addr), ip, INET_ADDRSTRLEN);  
		getnameinfo((struct sockaddr *)&server, sizeof(struct sockaddr), ip, sizeof(ip), NULL, 0, 0);

		printf("%s\n", ip);

		//Listen to incoming connections
		listen(s, 5);
		puts("Waiting for incoming connections...");

		//Accept one connection.
		if ((new_socket = accept(s, (struct sockaddr*)&client, &c)) == INVALID_SOCKET) {
			printf("accept failed with error code : %d", WSAGetLastError());
		}else {
			puts("Connection accepted\n-------------------");
		}

		//Chat loop.
		thread = CreateThread(NULL, 0, receive, (void*)&new_socket, 0, &thread_id);
		do {
			fgets(message, 255, stdin);
			if (send(new_socket, message, strlen(message), 0) == SOCKET_ERROR) {
				printf("Error at send(): %d", WSAGetLastError());
				break;
			}
		} while (1);

		closesocket(new_socket);
		closesocket(s);
		WSACleanup();
		free(message);

	}//Client Flow
	else {

		//inet_pton(AF_INET,ip,&server.sin_addr);
		getaddrinfo(ip,NULL,0,&server.sin_addr);

		if (connect(s,(struct sockaddr *)&server, sizeof(server)) < 0) {
			printf("Connection Error: %d", WSAGetLastError());
			closesocket(s);
		}else {
			printf("Connected @ %s", ip);
		}

		//Chat loop.
		thread = CreateThread(NULL, 0, receive, (void*)&s, 0, &thread_id);
		do {
			fgets(message, 255, stdin);
			if (send(s, message, strlen(message), 0) == SOCKET_ERROR) {
				printf("Error at send(): %d", WSAGetLastError());
				break;
			}
		} while (1);

	}
	printf("Shutting down...");
	free(ip);
	closesocket(s);
	WSACleanup();

	return 0;
}
