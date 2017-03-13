// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include <stdafx.h>
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
#define MAXTRY 5


#define DEFAULTUSER "username"
#define DEFAULTPASS "password"

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
	SOCKET conn_sock;

	sockaddr_in clientAddr;
	int clientAddrlen = sizeof(clientAddr);

	while (true) {
		conn_sock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrlen);
		printf("Accept new connection from :[%s:%d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		_beginthreadex(0, 0, fileServe, (void *)conn_sock, 0, 0); //start thread
	}
	closesocket(listenSock);
	WSACleanup();
	return 0;
}

unsigned __stdcall fileServe(void *param) {
	char buffer[BUFFSIZE];
	memset(buffer, '0', BUFFSIZE);
	SOCKET conn_sock = (SOCKET)param;
	int returnByte;
	int attemt_count = 0;
	int user_is_login = 0;
	char username[USERLENGHT+1];
	memset(username, 0, USERLENGHT + 1);
	char password[PASSLENGHT+1];
	memset(password, 0, PASSLENGHT + 1);
	char action_code[5];

	printf("Server accepted connection from client\n");
	snprintf(buffer, BUFFSIZE, "%dConnection establish login account for service\n", 332);
	send(conn_sock, buffer, BUFFSIZE, 0);

	//action indentify
	while (attemt_count < MAXTRY) {
		//read actioncode
		recv(conn_sock, buffer, BUFFSIZE, 0);
		strncpy(action_code, buffer, 4);
		action_code[4] = '\0';

		memset(password, 0, PASSLENGHT);

		switch (GetClientAction(action_code))
		{
		// actioncode is USER
		case 1:
			strncpy(username, buffer + 5, USERLENGHT);
			username[strlen(username)] = '\0';
			attemt_count++;

			//user try to login while still on session
			if (user_is_login) {
				snprintf(buffer, BUFFSIZE, "%dService in user: %s session, to access other account log out first", 230, username);
				send(conn_sock, buffer, BUFFSIZE, 0);
				continue;
			}

			//verify username in database
			if (strncmp(username, DEFAULTUSER, USERLENGHT) == 0) {
				//username request match user
				snprintf(buffer, BUFFSIZE, "%dLog in account: %s! Enter password\n", 331, username);
				send(conn_sock, buffer, BUFFSIZE, 0);
				continue;
			}

			//username not match
			snprintf(buffer, BUFFSIZE, "%dLogin error: not found user %s\n", 430, username);
			send(conn_sock, buffer, BUFFSIZE, 0);
			username[0] = '\0';
			continue;
		//action code is PASS
		case 2:
			strncpy(password, buffer + 5, PASSLENGHT);
			password[PASSLENGHT] = '\0';
			attemt_count++;

			//user send password while still on session
			if (user_is_login) {
				snprintf(buffer, BUFFSIZE, "%dService ready to user: %s session", 230, username);
				send(conn_sock, buffer, BUFFSIZE, 0);
				continue;
			}

			//verify password for user in database
			if (strncmp(password, DEFAULTPASS, PASSLENGHT) == 0 && strncmp(username, DEFAULTUSER, USERLENGHT) == 0) {
				//password match user's account
				snprintf(buffer, BUFFSIZE, "%dLog in %s: Successfull! Service ready\n", 230, username);
				send(conn_sock, buffer, BUFFSIZE, 0);

				//reset attempcount, set user in session variable
				attemt_count = 0;
				user_is_login = 1;
				continue;
			}

			//password not match
			snprintf(buffer, BUFFSIZE, "%dLogin error: password not match %s\n", 430, username);
			send(conn_sock, buffer, BUFFSIZE, 0);
			continue;
		//user sell terminate service
		case 0:
			printf("Service terminate!\n");
			closesocket(conn_sock);
			return 0;

		//log out
		case 9:
			//user send logout without being in session
			if (!user_is_login) {
				snprintf(buffer, BUFFSIZE, "%dLogout error: User not login\n", 451);
				send(conn_sock, buffer, BUFFSIZE, 0);
				continue;
			}

			//user in session, logout
			user_is_login = 0;
			attemt_count = 0;
			memset(username, 0, USERLENGHT + 1);
			snprintf(buffer, BUFFSIZE, "%dLogout successful\n", 231);
			send(conn_sock, buffer, BUFFSIZE, 0);
			continue;
		
		default:
			strncpy(buffer, "451Command not recognize\n", BUFFSIZE);
			send(conn_sock, buffer, BUFFSIZE, 0);
			continue;
		}
		

	}
}

