#include <stdio.h>
#include "pieces.c"

saveToFile(char *filePath, char *torrentPath,char *data,  unsigned char *hash, int pieceLength){
	int index = findHash(hash, torrentPath);
	File *file = fopen(filePath, "a+");
	fseek(file, index*pieceLength, SEEK_SET);
	fprintf(file, "%s", data);
	fclose(file);
}