#include "Utils.h"



int _tmain(int argc, _TCHAR* argv1[])
{
	//char *server = "127.0.0.1";
	//char *argv[] = { "tftp", "45.116.12.104", "put", "1234.txt" };
	//argc = 4;

	if (argc != 5) {
		cout << "Argc Error! TYPE: \"TFTP.exe SERVER_IP PORT OPTION(get, put) filename\"" << endl;
		return 0;
	}
	char *argv[5];
	for (int i = 0; i < argc; i++) {
		int iLength;
		iLength = WideCharToMultiByte(CP_ACP, 0, argv1[i], -1, NULL, 0, NULL, NULL);
		//½«tcharÖµ¸³¸ø_char
		argv[i] = (char *)malloc(iLength + 1);
		WideCharToMultiByte(CP_ACP, 0, argv1[i], -1, argv[i], iLength, NULL, NULL);
	}
	char *server = (char *)argv[1];

	int port = atoi(argv[2]);
	WORD myVersionRequest;
	WSADATA wsaData;
	myVersionRequest = MAKEWORD(1, 1);
	int ret;
	ret = WSAStartup(myVersionRequest, &wsaData);
	if (ret != 0) {
		cout << "Socket Opened Error!" << endl;
		return 0;
	}
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket == INVALID_SOCKET) {
		cout << "Socket() failed!" << endl;
		WSACleanup();
		return 0;
	}
	/*
	int imode = 1;
	ret = ioctlsocket(clientSocket, FIONBIO, (u_long *)&imode);
	if (ret == SOCKET_ERROR) {
	cout << "ioctlsocket() failed!" << endl;
	closesocket(clientSocket);
	WSACleanup();
	return 0;
	}
	*/

	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr(server);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	/*
	int len = sizeof(addr);
	char *data;
	data = (char *)malloc(bufferSize + 4);
	unsigned short index = 0;
	*/


	if (strcmp(argv[3], "put") == 0 || strcmp(argv[3], "PUT") == 0) {
		ret = putFile(clientSocket, addr, argv[4]);
	}
	else if (strcmp(argv[3], "get") == 0 || strcmp(argv[3], "GET") == 0) {
		ret = getFile(clientSocket, addr, argv[4]);
	}
	else {
		cout << "Unknown command!" << endl;
	}


	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
