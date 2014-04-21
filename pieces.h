#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

unsigned char *getHash(char torrentPath[], int piece);
int findHash(unsigned char hash[], char filePath[]); // 1 == found, 0==not found, -1 = error

#endif