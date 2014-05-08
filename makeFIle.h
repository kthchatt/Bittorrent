#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct tFile{
	char *path;
	int length;
}tFile;

typedef struct Torrent{
	char *announce, 		// tracker url 
		 *announceList,	
		 *dirName; 			// file/dir name
	int	pieceLength;		// bytes per piece
	tFile *files;		
	unsigned char *pieces; 	// hash list		
} Torrent;

// make new torrent file
int createTorrent(Torrent t);
// save recieved data to file 
int saveToFile(char *filePath, char *torrentPath, char *data,  unsigned char *hash, int pieceLength);
// get data from file matching the hash
char *readFromFile(char *filePath, char *torrentPath, unsigned char *hash, int pieceLength);

#endif