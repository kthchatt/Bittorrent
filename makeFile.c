#include "pieces.c"
#include "makeFile.h"

int saveToFile(char *filePath, char *torrentPath, char *data,  unsigned char *hash, int pieceLength){
	int index = findHash(hash, torrentPath);
	FILE *file;
	fopen(filePath, "wb");
	fseek(file, index*pieceLength, SEEK_SET);
	fprintf(file, "%s", data);
	fclose(file);
	return 0;
}

char *readFromFile(char *filePath, char *torrentPath, unsigned char *hash, int pieceLength){
	int index = findHash(hash, torrentPath);
	char *piece;

	FILE *file;
	fopen(filePath, "rb");
	fseek(file, index*pieceLength, SEEK_SET);
	fread(piece, pieceLength, 1, file);
	fclose(file);
	return piece;
}
