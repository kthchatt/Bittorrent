#ifndef _TRANSFER_H
#define _TRANSFER_H

#define PORT 5703

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>

int recievePiece(char *filePath, unsigned char *pieceHash, int pieceSize, int pieceIndex); 			// 1 == SUCCESS, 0 == ERROR
int sendPiece(char *filePath, char *destIP, int pieceSize, int pieceIndex); // 1 == SUCCESS, 0 == ERROR

#endif
