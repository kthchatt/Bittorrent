#include "torrent.h"

int createTorrent(Torrent t){

}

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
	// read file into buffer
	fread(buffer, fileLen, 1, file);
	// get length of all pieces + ":pieces"
	pLen = strlen(strstr(buffer, ":pieces"));
	// allocate memmory for file without pieces
	buffer = realloc(buffer, fileLen-pLen+1);
	// read file without pieces into buffer
	fread(buffer, fileLen-pLen, 1, file);
	// Calculate info hash
	SHA1(buffer, fileLen-pLen+1, infoHash);

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
		// copy torrent to static path
		execl("/bin/cp", "/bin/cp", torrentPath, destination, (char *)0);
	}else{
		wait(0);
		// get infohash from torrent file
		strcpy(infoHash, getInfoHash(destination));
		// get trackers from torrent file
		decode_bencode(torrentPath, m);
		// add trackers to array with length MAX_TRACKERS
		for(i=0; i<MAX_TRACKERS, i++)
			strcpy(trackers[i], m._announce_list[i]);
		// announce to tracker, init transfer
		track(infoHash, trackers);
	}
	return 0;
}