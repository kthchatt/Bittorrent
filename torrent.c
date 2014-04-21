#include <stdio.h>

typedef struct tFile(){
	char *path;
	int length;
}tFile;

typedef struct Torrent(){
	char *announce; 		// tracker url
	char *info;		
	char *name; 			// file/dir name
	unsigned char *pieces; 	// hash list
	int	pieceLength;		// bytes per piece
	int fileSize;			// filesize
	tFile file;				
} Torrent;

int createTorrent(Torrent t){
	File *nFile;
	nFile = fopen("test.torrent", "a+");
}