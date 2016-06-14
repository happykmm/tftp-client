#include "Utils.h"

int bufferSize = 512;


int getFile(SOCKET socket, SOCKADDR_IN addr, char *filename)
{
	char *mode = "octet";
	char *out = (char *)malloc(MAX_BUFFER_LENGTH);
	memset(out, 0, MAX_BUFFER_LENGTH);
	int timeout = 5;
	Package *op = (Package *)out;
	op->opCode = htons(TFTP_OP_READ);
	int offset = 2;
	sprintf(out + offset, "%s", filename);
	offset += strlen(filename) + 1;
	sprintf(out + offset, "%s", mode);
	offset += strlen(mode) + 1;
	sprintf(out + offset, "blksize");
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "%d", bufferSize);
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "timeout");
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "%d", timeout);
	offset += strlen(out + offset) + 1;


	//sprintf(out, "%02u%s\0%s\0timeout\0%d\0blksize\0%d\0", (unsigned int)TFTP_OP_READ, filename, mode, timeout, bufferSize);
#ifdef _DEBUG_MODE
	for (int i = 0; i < offset; i++) {
		printf("%hhu ", out[i]);
	}
#endif

	int ret = mySend(socket, out, offset, addr);
	free(out);


	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Create file \"%s\" error.\n", filename);
		return -2;
	}

	int len = sizeof(addr);
	char *data;
	data = (char *)malloc(bufferSize + 4);
	unsigned short index = 0;

	while (1) {
		memset(data, 0, bufferSize + 4);
		ret = recvfrom(socket, data, bufferSize + 4, 0, (SOCKADDR *)&addr, &len);
		if (ret == SOCKET_ERROR) {
			cout << "recvform() failed!" << endl;
			break;
		}

		Package *op = (Package *)data;
		int opCode = ntohs(op->opCode);
#ifdef _DEBUG_MODE
		cout << "------------" << endl;
		cout << "recv(): " << ret << endl;
		cout << "opCode: " << opCode << endl;
#endif
		if (opCode == TFTP_OP_DATA) {
			index = ntohs(op->code);
			fwrite(data + 4, 1, ret - 4, fp);
			int sendRet = sendACK(socket, index, addr);
			if (sendRet == SOCKET_ERROR) {
				break;
			}
			if (ret < bufferSize + 4) {
				cout << "Get successfully!" << endl;
				break;
			}
		}
		else if (opCode == TFTP_OP_ERR) {
			handleError(data, ret);
			break;
		}
		else {
			cout << "Unknown OPCODE!" << endl;
			break;
		}
	}
	fclose(fp);
	return 1;
}

int putFile(SOCKET socket, SOCKADDR_IN addr, char *filename)
{
	char *mode = "octet";
	char *out = (char *)malloc(MAX_BUFFER_LENGTH);
	memset(out, 0, MAX_BUFFER_LENGTH);
	int timeout = 5;
	Package *op = (Package *)out;
	op->opCode = htons(TFTP_OP_WRITE);
	int offset = 2;
	sprintf(out + offset, "%s", filename);
	offset += strlen(filename) + 1;
	sprintf(out + offset, "%s", mode);
	offset += strlen(mode) + 1;
	sprintf(out + offset, "blksize");
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "%d", bufferSize);
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "timeout");
	offset += strlen(out + offset) + 1;
	sprintf(out + offset, "%d", timeout);
	offset += strlen(out + offset) + 1;


	//sprintf(out, "%02u%s\0%s\0timeout\0%d\0blksize\0%d\0", (unsigned int)TFTP_OP_READ, filename, mode, timeout, bufferSize);
#ifdef _DEBUG_MODE
	for (int i = 0; i < offset; i++) {
		printf("%hhu ", out[i]);
	}
#endif

	int ret = mySend(socket, out, offset, addr);
	free(out);


	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Read file \"%s\" error.\n", filename);
		return -2;
	}

	int len = sizeof(addr);
	char *data;
	data = (char *)malloc(bufferSize + 4);
	unsigned short index = 0;

	while (1) {
		memset(data, 0, bufferSize + 4);
		ret = recvfrom(socket, data, bufferSize + 4, 0, (SOCKADDR *)&addr, &len);
		if (ret == SOCKET_ERROR) {
			cout << "recvform() failed!" << endl;
			break;
		}


		Package *op = (Package *)data;
		int opCode = ntohs(op->opCode);
#ifdef _DEBUG_MODE
		cout << "------------" << endl;
		cout << "recv(): " << ret << endl;
		cout << "opCode: " << opCode << endl;
#endif
		if (opCode == TFTP_OP_ACK) {
			index = ntohs(op->code);
			fseek(fp, (int)index * bufferSize, 0);

			op->opCode = htons(TFTP_OP_DATA);
			op->code = htons(index + 1);
			ret = fread(data + 4, 1, bufferSize, fp) + 4;
			if (ret == 4) {
				cout << "Sent successfully!" << endl;
				break;
			}
			int sendRet = mySend(socket, data, ret, addr);
			if (sendRet == SOCKET_ERROR) {
				break;
			}
		}
		else if (opCode == TFTP_OP_ERR) {
			handleError(data, ret);
			break;
		}
		else {
			cout << "Unknown OPCODE!" << endl;
			break;
		}
	}
	fclose(fp);
	return 1;
}

int mySend(SOCKET socket, char *message, int length, SOCKADDR_IN addr)
{
	int ret = sendto(socket, message, length, 0, (SOCKADDR *)&addr, sizeof(addr));
	if (ret == SOCKET_ERROR) {
		cout << "sendto() failed!" << endl;
		return SOCKET_ERROR;
	}

#ifdef _DEBUG_MODE
	cout << "sendto(): " << ret << endl;
#endif
	return ret;
}

int sendACK(SOCKET socket, unsigned short index, SOCKADDR_IN addr)
{
	char x[4];
	Package *pac = (Package *)x;
	pac->opCode = htons(TFTP_OP_ACK);
	pac->code = htons(index);
	int ret = mySend(socket, x, 4, addr);
	if (ret == SOCKET_ERROR) {
		cout << "ack error!" << endl;
	}
	return ret;
}

int handleError(char *data, int length)
{
	Package *op = (Package *)data;
	unsigned short errCode = ntohs(op->code);
	cout << "Error: " << data + 4 << endl;
	return 1;
}

