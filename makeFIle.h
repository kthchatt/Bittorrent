#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>

int saveToFile(char *filePath, char *torrentPath,char *data,  unsigned char *hash, int pieceLength);
char *readFromFile(char *filePath, char *torrentPath, unsigned char *hash, int pieceLength);

#endif