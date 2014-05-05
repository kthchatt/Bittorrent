#ifndef _torrent_h
#define _torrent_h

#include <stdio.h>
#include <openssl/sha>
#include "tracker.h"
#define MAX_TRACKERS 4


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

// make new torrent file
int createTorrent(Torrent t);
// get info hash from torrent file
char *getInfoHash(char *torrentPath);
// copy torrent to directory, announce to tracker and init transfer
int addTorrent(char *torrentPath);

#endif 