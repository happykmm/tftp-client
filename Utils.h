#ifndef UTILS_H
#define UTILS_H

#include "stdafx.h"
#include "Protocal.h"
#include <time.h>


int mySend(SOCKET socket, char *message, int length, SOCKADDR_IN addr);
int sendACK(SOCKET socket, unsigned short index, SOCKADDR_IN addr);
int getFile(SOCKET socket, SOCKADDR_IN addr, char *filename);
int putFile(SOCKET socket, SOCKADDR_IN addr, char *filename);
int handleError(char *data, int length);




#endif