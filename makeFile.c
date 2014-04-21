#include "pieces.c"

int saveToFile(char *filePath, char *torrentPath, char *data,  unsigned char *hash, int pieceLength){
	int index = findHash(hash, torrentPath);
	File *file = fopen(filePath, "a+");
	fseek(file, index*pieceLength, SEEK_SET);
	fprintf(file, "%s", data);
	fclose(file);
	return 0;
}

char *readFromFile(char *filePath, char *torrentPath, unsigned char *hash, int pieceLength){
	int index = findHash(hash, torrentPath);
	char *buffer;

	File *file = fopen(filePath, "rb");
	fseek(file, index*pieceLength, SEEK_SET);
	fread(buffer, pieceLength, 1, file);

	return &buffer;
}	