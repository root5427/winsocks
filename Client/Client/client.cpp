#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

int main(int argc, char *argv[]) {
	WSADATA wsaData;
	int iRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (iRes != 0) {
		printf("WSAStartup failed: %d\n", iRes);
		return 1;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iRes = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iRes != 0) {
		printf("getaddrinfo failed: %d\n", iRes);
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 2;
	}

	iRes = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iRes == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 3;
	}

	int recvbuflen = DEFAULT_BUFLEN;

	const char *sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];

	iRes = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iRes == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 4;
	}

	printf("Bytes sent: %ld\n", iRes);

	iRes = shutdown(ConnectSocket, SD_SEND);
	if (iRes == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 5;
	}

	do {
		iRes = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iRes > 0) {
			printf("Bytes received: %d\n", iRes);
		}
		else if (iRes == 0) {
			printf("Connection closed\n");
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
		}
	} while (iRes > 0);

	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}