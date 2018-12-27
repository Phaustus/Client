#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <fstream>
#include <cstdlib>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

char Username[DEFAULT_BUFLEN];
char Password[DEFAULT_BUFLEN];

int makeSocket(SOCKET *sock) {

	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*sock == INVALID_SOCKET)
	{
		printf("Error at socket().\n");
		WSACleanup();
		return 1;
	}
	else
	{
		return 0;
		printf("\nSocket is succesfully created.\n");
	}
}

void makeServer(struct sockaddr_in *server, const char adress[]) {
	server->sin_addr.s_addr = inet_addr(adress);
	server->sin_family = AF_INET;
	server->sin_port = htons(DEFAULT_PORT);
}

int makeConnection(SOCKET sock, struct sockaddr_in server) {
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		perror("\nConnect failed. Error\n");
		closesocket(sock);
		WSACleanup();
		return 1;
	}
	puts("Connected\n");
	return 0;
}

int sendFun(SOCKET sock, char message[DEFAULT_BUFLEN]) {
	if (send(sock, message, strlen(message), 0) < 0) {
		puts("Send failed");
		return 1;
	}
	return 0;
}

FILE *safeFileOpen(char *filename, const char *mode, int error_code) {
	FILE *fp;
	if (int lol = fopen_s(&fp, "C:\bla.txt", "r") != 0)
	{
		printf("Can't open '%s'!\n", filename);
		exit(error_code);
	}
	return fp;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("USAGE: %s filename\n", argv[0]);
		exit(11);
	}

	// Initialize Winsock, request the Winsock 2.2
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup().\n");
		return 1;
	}
	else
	{
		printf("WSA library is succesfully initilized.\n");
	}

	//Initialization of server structure used by bind
	struct sockaddr_in server;
	char confirm_msg[DEFAULT_BUFLEN] = "\0";
	char server_msg[DEFAULT_BUFLEN] = "\0";
	char message[DEFAULT_BUFLEN];
	char *in_filename = argv[1];

	//Making socket
	SOCKET sock;
	if (makeSocket(&sock) == INVALID_SOCKET) {
		return 0;
	}
	makeServer(&server, "192.168.0.39");

	//Connect to remote server
	if (makeConnection(sock, server) == 1) {
		return 0;
	}
	//dovde valjda doradjeno stvarno // ma sve radi sada valjda
	// UNOS USERNAME-a
	while (1) {
		puts("Username: ");
		gets_s(Username, sizeof(Username));

		//SLANJE USERA
		sendFun(sock, Username);

		puts("Password: ");
		gets_s(Password, sizeof(Password));

		sendFun(sock, Password);
		recv(sock, confirm_msg, DEFAULT_BUFLEN, 0); //Prima potvrdu da je pogodio user i pass
		int i = atoi(confirm_msg);
		if (i == 1) {
			puts("Connected\n");
			break;
		}
		puts("Wrong username or password\n");
	}

	puts("Data sending: \n");

	FILE *in = safeFileOpen(in_filename, "r", 1);
	long int size = ftell(in); //uzima duzinu fajla
	int n = size / DEFAULT_BUFLEN;
	int i;
	int fs_block;
	for (i = 0; i < n + 1; i++)
	{
		while ((fs_block = fread(message, sizeof(char), DEFAULT_BUFLEN, in)) > 0) {
			if (send(sock, message, fs_block, 0) < 0) {
				printf("ERROR: Failed to send file %s. \n", in_filename);
				return 1;
			}
		}
	}
	printf("OK FILE %s from client was sent! \n", in_filename);

	fclose(in);
	closesocket(sock);
	WSACleanup();

	return 0;
}