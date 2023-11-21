#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define BUFFER_SIZE 512

struct Merenje
{
	char nazivGrada[20];
	short indeksKvalitetaVazduha;
};

#define PORT1 19000
#define PORT2 19001
#define MAX_CLIENTS 2

int main()
{
	int iResult;
	char bafer[BUFFER_SIZE], bafer1[BUFFER_SIZE], bafer2[BUFFER_SIZE];
	int connectedClients = 0;
	int lastIndex = 0;
	WSADATA wsaData;

	Merenje spisak[10];

	// reset strutkrue
	for (int i = 0; i < 10; i++)
	{
		memset(spisak[i].nazivGrada, 0, 20); //jer je nazivgrada max duzine 20, u strukturi tako pise
		spisak[i].indeksKvalitetaVazduha = 0;
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Error starting WSA (%d)!\n", WSAGetLastError());
		return 1;
	}

	sockaddr_in serverAddress[2];

	memset(serverAddress, 0, sizeof(serverAddress)); //ne zaboraviti ovo
	memset(bafer, 0, BUFFER_SIZE);
	memset(bafer2, 0, BUFFER_SIZE);

	//prva merna stanica
	serverAddress[0].sin_family = AF_INET;
	serverAddress[0].sin_addr.s_addr = INADDR_ANY;
	serverAddress[0].sin_port = htons(PORT1);

	// druga merna stanica
	serverAddress[1].sin_family = AF_INET;
	serverAddress[1].sin_addr.s_addr = INADDR_ANY;
	serverAddress[1].sin_port = htons(PORT2);

	// kreiranje uticnica
	SOCKET serverSocket[2];
	serverSocket[0] = INVALID_SOCKET;
	serverSocket[0] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (serverSocket[0] == INVALID_SOCKET)
	{
		printf("Error creating socket (%d)!\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// bind serverskih uticnica
	iResult = bind(serverSocket[0], (SOCKADDR*)&serverAddress[0], sizeof(serverAddress[0]));

	if (iResult == SOCKET_ERROR)
	{
		printf("Error binding socket 1 (%d)!\n", WSAGetLastError());
		closesocket(serverSocket[0]);
		WSACleanup();
		return 1;
	}

	// druga uticnica
	serverSocket[1] = INVALID_SOCKET;
	serverSocket[1] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (serverSocket[1] == INVALID_SOCKET)
	{
		printf("Error creating socket 2 (%d)!\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(serverSocket[1], (SOCKADDR*)&serverAddress[1], sizeof(serverAddress[1]));

	if (iResult == SOCKET_ERROR)
	{
		printf("Error binding socket 2 (%d)!\n", WSAGetLastError());
		closesocket(serverSocket[0]);
		closesocket(serverSocket[1]);
		WSACleanup();
		return 1;
	}

	printf("\nUDP Server started...\n\n");

	int connected = 0;

	// KLIJENTI
	sockaddr_in clientAddr[2];
	int clientAddrSize[2] = { sizeof(struct sockaddr_in),  sizeof(struct sockaddr_in) }; //stavljamo i na 0 i na 1 mjesto sizeof strukture
																						//cak mislim da nema potrebe za ovo, duzina je ista

	memset(&clientAddr, 0, sizeof(sockaddr_in)); //stavljamo na nulu adrese, inicijalizacija
	memset(bafer, 0, BUFFER_SIZE);

	// polling model
	do
	{
		iResult = recvfrom(serverSocket[connected], bafer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddr[connected], &clientAddrSize[connected]);

		if (iResult != SOCKET_ERROR)
		{
			printf("\nNovi klijent: %s : %d\n", inet_ntoa(clientAddr[connected].sin_addr), ntohs(clientAddr[connected].sin_port));

			unsigned long mode = 1;
			iResult = ioctlsocket(serverSocket[connected], FIONBIO, &mode);
			if (iResult != NO_ERROR)
				printf("ioctlsocket failed with error: %ld\n", iResult);

			connected++; //na pocetku je nula, gore iznad do, i onda povecava dok ne dodje do 2, to je dole implementirano
			//dakle samo 2 klijenta u ovom slucaju
			//na kraju ovog do, vracamo na 0
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				Sleep(2000);
			}
			else
			{
				printf("error connection with client error: %d\n", WSAGetLastError());
				closesocket(serverSocket[connected]);
				WSACleanup();
				return 1;
			}

		}

		if (connected < 2)
		{
			continue; //preskace ostatak loopa (ostatak programa) dok se ne konektuje i drugi
		}

		// poruka uspesna prijava
		strcpy_s(bafer, "Uspesna prijava");

		iResult = sendto(serverSocket[0], bafer, strlen(bafer), 0, (SOCKADDR*)&clientAddr[0], clientAddrSize[0]);
		if (iResult == SOCKET_ERROR)
		{
			printf("Error sending success message to socket 1 (%d)!\n", WSAGetLastError());
			closesocket(serverSocket[0]);
			WSACleanup();
			return 1;
		}

		iResult = sendto(serverSocket[1], bafer, strlen(bafer), 0, (SOCKADDR*)&clientAddr[1], clientAddrSize[1]);
		if (iResult == SOCKET_ERROR)
		{
			printf("Error sending success message to socket 2 (%d)!\n", WSAGetLastError());
			closesocket(serverSocket[0]);
			closesocket(serverSocket[1]);
			WSACleanup();
			return 1;
		}

		do
		{
			
		} while (true);
		connected = 0;
	} while (true);

	// clean up
	iResult = closesocket(serverSocket[0]);

	if (iResult == SOCKET_ERROR)
	{
		printf("Error closing socket 1 (%d)!\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = closesocket(serverSocket[1]);

	if (iResult == SOCKET_ERROR)
	{
		printf("Error closing socket 2 (%d)!\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	WSACleanup();

	return 0;
}


KLIJENT

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define BUFFER_SIZE 512
#define SERVER_ADDRESS "127.0.0.1"

struct Merenje
{
	char nazivGrada[20];
	short indeksKvalitetaVazduha;
};

int main()
{
	int iResult;
	short port;
	char bafer[BUFFER_SIZE];
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Error starting WSA (%d)!\n", WSAGetLastError());
		return 1;
	}

	SOCKET serverSocket = INVALID_SOCKET;

	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (serverSocket == INVALID_SOCKET)
	{
		printf("Error creating socket (%d)!\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAdress;
	int serverAddrLen = sizeof(serverAdress);

	memset(&serverAdress, 0, sizeof(serverAdress));
	serverAdress.sin_family = AF_INET;
	serverAdress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

	printf("Unesite port (19000 ili 19001): ");
	gets_s(bafer, BUFFER_SIZE);

	port = atoi(bafer);

	serverAdress.sin_port = htons(port);

	strcpy_s(bafer, "Prijava");
	iResult = sendto(serverSocket, bafer, strlen(bafer), 0, (SOCKADDR*)&serverAdress, sizeof(serverAdress));

	if (iResult == SOCKET_ERROR)
	{
		printf("Error sending data to socket (%d)!\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	// povratna poruka da je prijava uspesna
	iResult = recvfrom(serverSocket, bafer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAdress, &serverAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("Error receiving data from socket (%d)!\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	bafer[iResult] = '\0';

	printf("Server: %s\n", bafer);
	//ovde sto bude prije while tamo se desava u prvom do while-u
	//ugl uspostavljanje konekcije







