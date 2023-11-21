#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 19015
#define BUFFER_SIZE 256
#define Velicina 512

struct Merenje {
	char nazivGrada[21];
	short indexKvalitetaVazduha;
};

int main()
{
	SOCKET connectSocket = INVALID_SOCKET;

	int iResult;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);		// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("Poruka primljena od Servera: %s\n", dataBuffer);
	memset(&dataBuffer, 0, BUFFER_SIZE);

	Merenje jedanUzorak;
	short index;
	char end[Velicina];

	while (true)
	{
		// Unos potrebnih podataka koji ce se poslati serveru
		printf("Unesite naziv grada: ");
		gets_s(jedanUzorak.nazivGrada, 21);

		printf("Unesite index kvaliteta vazduha: ");
		scanf_s("%d", &index);
		getchar();    //pokupiti enter karakter iz bafera tastature
		jedanUzorak.indexKvalitetaVazduha = index;

		iResult = send(connectSocket, (char*)&jedanUzorak, (int)sizeof(jedanUzorak), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Message successfully sent. Total bytes: %ld\n", iResult);

		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)
        {
			dataBuffer[iResult] = '\0';
			printf("\nPrimljena poruka je: %s\n", dataBuffer);
        }

		printf("\nAko zelite da zavrsite unesite 'Kraj': \n");
		gets_s(end, Velicina);
		if (strcmp(end, "Kraj") == 0) {
			break;
		}
	}

	iResult = shutdown(connectSocket, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	closesocket(connectSocket);

	WSACleanup();

	return 0;
}