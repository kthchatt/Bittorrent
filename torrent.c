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
	fread(buffer, fileLen, 1, file);				
	pLen = strlen(strstr(buffer, ":info"));			
	buffer = strstr(buffer, "info");
	buffer = strstr(buffer, "d");
	buffer = realloc(buffer, pLen-5);		

	SHA1(buffer, pLen-5, infoHash);			// Calculate info hash

	fclose(file);
	return infoHash;
}

/*int addTorrent(char *torrentPath){
	char infoHash[20],
		 destination[] = "/my/path/t.torrent",
		 trackers[MAX_TRACKERS];
	torrent_info *m;
	int i;

	pid_t pid = fork();

	if(pid==0){
		execl("/bin/cp", "/bin/cp", torrentPath, destination, (char *)0);	// copy torrent to static path
	}else{
		wait(0);
		strcpy(infoHash, getInfoHash(destination));							// get infohash from torrent file
		decode_bencode(destination, m);										// get trackers from torrent file
		for(i=0; i<MAX_TRACKERS; i++)										// add trackers to array with length MAX_TRACKERS
			strcpy(trackers[i], m._announce_list[i]);						// announce to tracker, init transfer
		track(infoHash, trackers);											// magic
	}
	return 0;
}*/



void main(void)
{
	printf("IH: %s", getInfoHash("t.torrent"));
	fflush(stdout);
}