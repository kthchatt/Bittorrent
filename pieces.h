#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

unsigned char *getHash(char torrentPath[], int piece, int totalPieces);
int findHash(unsigned char hash[], char filePath[], int totalPieces); // returns hash index if found, 0==not found, -1 = error

#endif
