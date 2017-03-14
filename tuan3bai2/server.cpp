// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <process.h>
#include <ctype.h>
#include "mylib.h"
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define QUEUE_SIZE 10


unsigned __stdcall fileServe(void *param);

int main(int argc, char** argv) {
	char *USAGE = "USAGE: server -p portnumber";
	int Server_port;
	if (strcmp(argv[1], "-p") != 0) {
		printf("%s", USAGE);
		_getch();
		return 1;
	}

	Server_port = atoi(argv[2]);
	// window socket init
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(Server_port);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error: cant not bind socket to port\n");
		_getch();
		return 0;
	}

	if (listen(listenSock, QUEUE_SIZE)) {
		printf("Error: cant listen for socket\n");
		_getch();
		return 0;
	}

	printf("server started at [%s:%d]\n", SERVER_ADDR, Server_port);

	//new socket to client connect
	struct client_info_struct clients[FD_SETSIZE];
	SOCKET conn_sock;
	fd_set readfds, writefds;
	sockaddr_in client_addr;
	int ret, events, client_addr_len;

	for (int i = 0; i < FD_SETSIZE; i++) {
		memset(&clients[i], 0, sizeof (struct  client_info_struct));
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	timeval time_out_interval;
	time_out_interval.tv_sec = 10;
	time_out_interval.tv_usec = 0;

	while (true) {
		FD_SET(listenSock, &readfds);

		for (int i = 0; i < FD_SETSIZE; i++)
			if (clients[i].client_fd > 0)
				FD_SET(clients[i].client_fd, &readfds);
		writefds = readfds;

		events = select(0, &readfds, 0, 0, 0);
		if (events < 0) {
			printf("\nError! CAnnot poll socket: %d", WSAGetLastError);
		}

		//set newly connect socket into clients array
		if (FD_ISSET(listenSock, &readfds)) {
			client_addr_len = sizeof(client_addr);
			conn_sock = accept(listenSock, (sockaddr*)&client_addr, &client_addr_len);
			int i;

			printf("Server receive new connection from[%s:%d]\n",
				inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			for(i = 0; i < FD_SETSIZE; i++)
				if (clients[i].client_fd <= 0) {
					clients[i].client_fd = conn_sock;
					break;
				}
			if (i == FD_SETSIZE)
				printf("\n Too many clients\n");

			if (--events <= 0)
				continue;
		}

		for (int i = 0; i < FD_SETSIZE; i++) {
			if (clients[i].client_fd <= 0)
				continue;

			if (FD_ISSET(clients[i].client_fd, &readfds)) {
				ret = receive_data(clients[i].client_fd, clients[i].client_buffer, BUFFSIZE, 0);
				if (ret <= 0) {
					FD_CLR(clients[i].client_fd, &readfds);
					closesocket(clients[i].client_fd);
					memset(&clients[i], 0, sizeof(struct client_info_struct));
				}
				else if (ret > 0) {
					process_data(&clients[i]);
					send_data(clients[i].client_fd, clients[i].client_buffer, BUFFSIZE, 0);
				}
			}
			if (--events <= 0)
				continue;
		}
	}
	closesocket(listenSock);
	WSACleanup();
	return 0;
}



