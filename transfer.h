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

int prepareFile(char *filePath, int fileSize); // 0 == unable to open file, fileSize = bytes to be written, creates a file filled with 0's
int receivePiece(char *filePath, unsigned char *pieceHash, int pieceSize, int pieceIndex); // 0 == SUCCESS, -1 == ERROR, -2 == PIECE HASH MISMATCH
int sendPiece(char *filePath, char *destIP, int pieceSize, int pieceIndex); // 0 == SUCCESS, -1 == ERROR

#endif
