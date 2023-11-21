#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 19015
#define BUFFER_SIZE 256

// Structura kojom se simulira merenjS
struct Merenje {
	char nazivGrada[21];
	short indexKvalitetaVazduha;
};

int main()
{
	SOCKET listenSocket = INVALID_SOCKET;

	SOCKET acceptedSocket1 = INVALID_SOCKET;
	SOCKET acceptedSocket2 = INVALID_SOCKET;

	int iResult;

	char dataBuffer1[BUFFER_SIZE];
	char dataBuffer2[BUFFER_SIZE];

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}


	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

	Merenje* jedanUzorak1;
	Merenje* jedanUzorak2;
	int i = 0;	// brojac za prvog klijenta
	int x = 0;	// brojac za drugog klijenta

	Merenje prikuljeniPodaci1[4];
	memset(&prikuljeniPodaci1, 0, 4 * sizeof(Merenje));

	Merenje prikuljeniPodaci2[4];
	memset(&prikuljeniPodaci2, 0, 4 * sizeof(Merenje));


	sockaddr_in clientAddr1;	// za informacije o klijentu 1
	sockaddr_in clientAddr2;	// za informacije o klijentu 2
	int clientAddrSize = sizeof(struct sockaddr_in);

	do {
		acceptedSocket1 = accept(listenSocket, (struct sockaddr*)&clientAddr1, &clientAddrSize);
		if (acceptedSocket1 == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("\nFirst client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr1.sin_addr), ntohs(clientAddr1.sin_port));

		acceptedSocket2 = accept(listenSocket, (struct sockaddr*)&clientAddr2, &clientAddrSize);
		if (acceptedSocket2 == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			closesocket(acceptedSocket1);
			WSACleanup();
			return 1;
		}

		printf("\nSecond client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr2.sin_addr), ntohs(clientAddr2.sin_port));

		// Saljemo poruku da je povezivanje uspesno
		char s[33] = "Uspesno povezivanje sa serverom!";

		iResult = send(acceptedSocket1, s, sizeof(s), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			shutdown(acceptedSocket1, SD_BOTH);
			shutdown(acceptedSocket2, SD_BOTH);
			closesocket(acceptedSocket1);
			closesocket(acceptedSocket2);
		}

		iResult = send(acceptedSocket2, s, sizeof(s), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			shutdown(acceptedSocket1, SD_BOTH);
			shutdown(acceptedSocket2, SD_BOTH);
			closesocket(acceptedSocket1);
			closesocket(acceptedSocket2);
		}

		// Postavljanje uticnica namenjenih klijentima u neblokirajuci rezim
		unsigned long mode = 1; //non-blocking mode
		iResult = ioctlsocket(acceptedSocket1, FIONBIO, &mode);
		if (iResult != NO_ERROR)
			printf("ioctlsocket failed with error: %ld\n", iResult);

		iResult = ioctlsocket(acceptedSocket2, FIONBIO, &mode);
		if (iResult != NO_ERROR)
			printf("ioctlsocket failed with error: %ld\n", iResult);

		while (true)
		{
			
		}
	} while (true);

	iResult = shutdown(acceptedSocket1, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket1);
		closesocket(acceptedSocket2);
		WSACleanup();
		return 1;
	}

	iResult = shutdown(acceptedSocket2, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket1);
		closesocket(acceptedSocket2);
		WSACleanup();
		return 1;
	}

	closesocket(listenSocket);
	closesocket(acceptedSocket1);
	closesocket(acceptedSocket2);

	WSACleanup();

	return 0;
}



KLIJENT

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