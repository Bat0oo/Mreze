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
	memset(&prikuljeniPodaci1, 0, 4*sizeof(Merenje));

	Merenje prikuljeniPodaci2[4];
	memset(&prikuljeniPodaci2, 0, 4*sizeof(Merenje));


	sockaddr_in clientAddr1;	// za informacije o klijentu 1
	sockaddr_in clientAddr2;	// za informacije o klijentu 2
	int clientAddrSize = sizeof(struct sockaddr_in);

do{
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

	while(true)
	{
		iResult = recv(acceptedSocket1, dataBuffer1, BUFFER_SIZE, 0);

		if (iResult > 0)
		{
			if (i < 4) {
				dataBuffer1[iResult] = '\0';

				jedanUzorak1 = (Merenje*)dataBuffer1;

				if (i == 0)
				{
					printf("\nNaziv grada: %s  \n", jedanUzorak1->nazivGrada);
					printf("Index kvaliteta vazduha: %d  \n", jedanUzorak1->indexKvalitetaVazduha);
					printf("_______________________________  \n");
					printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddr1.sin_addr), ntohs(clientAddr1.sin_port));

					strcpy_s(prikuljeniPodaci1[i].nazivGrada, jedanUzorak1->nazivGrada);
					prikuljeniPodaci1[i].indexKvalitetaVazduha = jedanUzorak1->indexKvalitetaVazduha;
					i++;
				}
				else
				{
					printf("\nTrenutno uneti podaci :\n");
					printf("\nNaziv grada: %s  \n", jedanUzorak1->nazivGrada);
					printf("Index kvaliteta vazduha: %d  \n", jedanUzorak1->indexKvalitetaVazduha);
					printf("_______________________________  \n");

					if (jedanUzorak1->indexKvalitetaVazduha > 0)
					{
						strcpy_s(prikuljeniPodaci1[i].nazivGrada, jedanUzorak1->nazivGrada);
						prikuljeniPodaci1[i].indexKvalitetaVazduha = jedanUzorak1->indexKvalitetaVazduha;
						i++;

						int j;
						printf("\nPodaci koje imamo sacuvano :\n");
						for (j = 0; j < i; j++) {
							printf("\nNaziv grada: %s  \n", prikuljeniPodaci1[j].nazivGrada);
							printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci1[j].indexKvalitetaVazduha);
							printf("_______________________________  \n");
						}

						int k, l;
						int najgori = 0;
						int index;
						for (k = 0; k < i; k++)
						{
							int najgori = prikuljeniPodaci1[k].indexKvalitetaVazduha;
							index = 0;
							for (l = k + 1; l < i; l++)
							{
								if (prikuljeniPodaci1[l].indexKvalitetaVazduha > najgori)
								{
									najgori = prikuljeniPodaci1[l].indexKvalitetaVazduha;
									index = l;
								}
							}

							if (k = i)
							{
								printf("\nPodaci o najzagadjenijem gradu :\n");
								printf("\nNaziv grada: %s  \n", prikuljeniPodaci1[index].nazivGrada);
								printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci1[index].indexKvalitetaVazduha);
								printf("_______________________________  \n");
							}
						}

						printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddr1.sin_addr), ntohs(clientAddr1.sin_port));

					}
					else if (jedanUzorak1->indexKvalitetaVazduha < 0) {
						char s[10] = "ODBACENO!";

						iResult = send(acceptedSocket1, s, sizeof(s), 0);
						if (iResult == SOCKET_ERROR)
						{
							printf("send failed with error: %d\n", WSAGetLastError());
							shutdown(acceptedSocket1, SD_BOTH);
							shutdown(acceptedSocket2, SD_BOTH);
							closesocket(acceptedSocket1);
							closesocket(acceptedSocket2);
							break;
						}

						Sleep(3000);
					}
				}

				char s[10] = "UNETO!";

				iResult = send(acceptedSocket1, s, sizeof(s), 0);
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					shutdown(acceptedSocket1, SD_BOTH);
					shutdown(acceptedSocket2, SD_BOTH);
					closesocket(acceptedSocket1);
					closesocket(acceptedSocket2);
					break;
				}
			}
			else {
				char s[10] = "ODBACENO!";

				iResult = send(acceptedSocket1, s, sizeof(s), 0);
				if (iResult == SOCKET_ERROR)
				{
					printf("send failed with error: %d\n", WSAGetLastError());
					shutdown(acceptedSocket1, SD_BOTH);
					shutdown(acceptedSocket2, SD_BOTH);
					closesocket(acceptedSocket1);
					closesocket(acceptedSocket2);
					break;
				}
			}
		}
		else if (iResult == 0)
		{
			// Connection was closed successfully
			printf("Connection with first client closed.\n");
			closesocket(acceptedSocket1);
			closesocket(acceptedSocket2);
			break;
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				//Sleep(3000);
			}
			else
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptedSocket1);
				closesocket(acceptedSocket2);
				break;
			}
		}
		
		iResult = recv(acceptedSocket2, dataBuffer2, BUFFER_SIZE, 0);
			if (iResult > 0)
			{
				if (x < 4) {
					dataBuffer2[iResult] = '\0';

					jedanUzorak2 = (Merenje*)dataBuffer2;

					if (x == 0)
					{
						printf("\nNaziv grada: %s  \n", jedanUzorak2->nazivGrada);
						printf("Index kvaliteta vazduha: %d  \n", jedanUzorak2->indexKvalitetaVazduha);
						printf("_______________________________  \n");
						printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddr2.sin_addr), ntohs(clientAddr2.sin_port));

						strcpy_s(prikuljeniPodaci2[x].nazivGrada, jedanUzorak2->nazivGrada);
						prikuljeniPodaci2[x].indexKvalitetaVazduha = jedanUzorak2->indexKvalitetaVazduha;
						x++;
					}
					else
					{
						printf("\nTrenutno uneti podaci :\n");
						printf("\nNaziv grada: %s  \n", jedanUzorak2->nazivGrada);
						printf("Index kvaliteta vazduha: %d  \n", jedanUzorak2->indexKvalitetaVazduha);
						printf("_______________________________  \n");

						if (jedanUzorak2->indexKvalitetaVazduha > 0) 
						{

							strcpy_s(prikuljeniPodaci2[x].nazivGrada, jedanUzorak2->nazivGrada);
							prikuljeniPodaci2[x].indexKvalitetaVazduha = jedanUzorak2->indexKvalitetaVazduha;
							x++;

							int u;
							printf("\nPodaci koje imamo sacuvano :\n");
							for (u = 0; u < x; u++) {
								printf("\nNaziv grada: %s  \n", prikuljeniPodaci2[u].nazivGrada);
								printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci2[u].indexKvalitetaVazduha);
								printf("_______________________________  \n");
							}

							int k, l;
							int najgori = 0;
							int index;
							for (k = 0; k < x; k++)
							{
								int najgori = prikuljeniPodaci2[k].indexKvalitetaVazduha;
								index = 0;
								for (l = k + 1; l < x; l++)
								{
									if (prikuljeniPodaci2[l].indexKvalitetaVazduha > najgori)
									{
										najgori = prikuljeniPodaci2[l].indexKvalitetaVazduha;
										index = l;
									}
								}

								if (k = x)
								{
									printf("\nPodaci o najzagadjenijem gradu :\n");
									printf("\nNaziv grada: %s  \n", prikuljeniPodaci2[index].nazivGrada);
									printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci2[index].indexKvalitetaVazduha);
									printf("_______________________________  \n");
								}
							}

							printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddr2.sin_addr), ntohs(clientAddr2.sin_port));

						}
						else if (jedanUzorak2->indexKvalitetaVazduha < 0) {
							char s[10] = "ODBACENO!";

							iResult = send(acceptedSocket2, s, sizeof(s), 0);
							if (iResult == SOCKET_ERROR)
							{
								printf("send failed with error: %d\n", WSAGetLastError());
								shutdown(acceptedSocket1, SD_BOTH);
								shutdown(acceptedSocket2, SD_BOTH);
								closesocket(acceptedSocket1);
								closesocket(acceptedSocket2);
								break;
							}

							Sleep(3000); // 3sekunde
						}

					}

					char s[10] = "UNETO!";

					iResult = send(acceptedSocket2, s, sizeof(s), 0);
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						shutdown(acceptedSocket1, SD_BOTH);
						shutdown(acceptedSocket2, SD_BOTH);
						closesocket(acceptedSocket1);
						closesocket(acceptedSocket2);
						break;
					}
				}
				else
				{
					char s[10] = "ODBACENO!";

					// Send message to clients using connected socket
					iResult = send(acceptedSocket2, s, sizeof(s), 0);
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						shutdown(acceptedSocket1, SD_BOTH);
						shutdown(acceptedSocket2, SD_BOTH);
						closesocket(acceptedSocket1);
						closesocket(acceptedSocket2);
						break;
					}
				}
			}
			else if (iResult == 0)
			{
				printf("Connection with client closed.\n");
				closesocket(acceptedSocket1);
				closesocket(acceptedSocket2);
				break;
			}
			else
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK) 
				{
					//Sleep(3000);
				}
				else
				{
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(acceptedSocket1);
					closesocket(acceptedSocket2);
					break;
				}
			}
	}
}while (true);

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