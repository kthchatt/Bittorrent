#include "pieces.c"
#include "makeFile.h"

int createTorrent(Torrent t){
	FILE *file = fopen("newTorrent.torrent", "wb");
	int i;

	fprintf(file, "d8:announce%d:%s13:announce-listll", strlen(t.announce), t.announce);
	for(i=0; i<sizeof(t.announceList)/sizeof(t.announceList[0])-1; i++)
		fprintf(file, "%d:%sel", strlen(t.announceList[i]), t.announceList[i]);
	fprintf(file, "%d:%see4:infod5:filesld", strlen(t.announceListi[i]), t.announceList[i]);
	
	for(i=0 i<sizeof(t.files)/sizeof(t.files[0]); i++)
		fprintf(file, "6:lengthi%de4:name%d:%s", t.files[i].length, strlen(t.files[i].name), t.files[i].name);
	
	fprintf(file, "12:piece lengthi%de6:pieces%d:", t.pieceLength, sizeof(t.pieces)/pieceLength);
	for(i=0 i<sizeof(t.pieces)/sizeof(t.pieces[0]); i++)
		fprintf(file, "%x", t.pieces[i]);

	fclose(file);
	return 1;
}

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