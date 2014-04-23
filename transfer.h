#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int recievePiece(char *filePath, int pieceSize, int pieceIndex); // 1 == SUCCESS, <= 0 = ERROR

#endif
