#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int main() {
	WSADATA wsaData;
	int iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	if (iRes != 0) {
		printf("WSAStartup failed: %d\n", iRes);
		return 1;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iRes = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iRes != 0) {
		printf("getaddrinfo failed: %d\n", iRes);
		WSACleanup();
		return 2;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}

	iRes = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iRes == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 4;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %d/\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 5;
	}

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: &%d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 6;
	}

	char recvbuf[DEFAULT_BUFLEN];
	int iSendRes;
	int recvbuflen = DEFAULT_BUFLEN;

	do {
		iRes = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iRes > 0) {
			printf("Bytes received: %d\n", iRes);

			iSendRes = send(ClientSocket, recvbuf, iRes, 0);
			if (iSendRes == SOCKET_ERROR) {
				printf("send failed:  %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 7;
			}
			printf("Bytes sent: %d\n", iSendRes);
		}
		else if (iRes == 0) {
			printf("Connection closing...\n");
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 7;
		}
	} while (iRes > 0);

	iRes = shutdown(ClientSocket, SD_SEND);
	if (iRes == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 8;
	}
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}