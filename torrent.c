#include "torrent.h"

char *getInfoHash(char *torrentPath){
	FILE *file = fopen(torrentPath, "rb");
	char *buffer, infoHash[20];
	int fileLen, pLen;
	if(!file) return "";

	// Get file length
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = malloc(fileLen+1);
	fread(buffer, fileLen, 1, file);				// read file into buffer
	pLen = strlen(strstr(buffer, ":pieces"));		// get length of all pieces + ":pieces"
	buffer = realloc(buffer, fileLen-pLen+1);		// allocate memmory for file without pieces
	fread(buffer, fileLen-pLen, 1, file);			// read file without pieces into buffer
	SHA1(buffer, fileLen-pLen+1, infoHash);			// Calculate info hash

	fclose(file);
	return infoHash;
}

int addTorrent(char *torrentPath){
	char infoHash[20],
		 destination[] = "/my/path/t.torrent",
		 trackers[MAX_TRACKERS];
	metainfodecode m;
	int i;

	pid_t pid = fork();

	if(pid==0){
		execl("/bin/cp", "/bin/cp", torrentPath, destination, (char *)0);	// copy torrent to static path
	}else{
		wait(0);
		strcpy(infoHash, getInfoHash(destination));							// get infohash from torrent file
		decode_bencode(destination, m);										// get trackers from torrent file
		for(i=0; i<MAX_TRACKERS, i++)										// add trackers to array with length MAX_TRACKERS
			strcpy(trackers[i], m._announce_list[i]);						// announce to tracker, init transfer
		track(infoHash, trackers);											// magic
	}
	return 0;
}