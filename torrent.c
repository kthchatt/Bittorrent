#include "torrent.h"

int createTorrent(Torrent t){
	//File *file = fopen("test.torrent", "wb");
}

char *getInfoHash(char *torrentPath){
	FILE *file = fopen(torrentPath, "rb");
	char *buffer, infoHash[20];
	int fileLen, pLen;
	if(!file) return "";

	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer = malloc(fileLen+1);
	fread(buffer, fileLen, 1, file);
	pLen = strlen(strstr(buffer, ":pieces"));
	buffer = realloc(buffer, fileLen-pLen+1);
	fread(buffer, fileLen-pLen, 1, file);

	SHA1(buffer, fileLen-pLen+1, infoHash);
	return infoHash;
}

int addTorrent(char *torrentPath){
	char infoHash[20],
		 destination[] = "/my/path/t.torrent",
		 trackers[MAX_TRACKERS] = {"udp://tracker.ccc.de:80/announce","","",""};

	pid_t pid = fork();

	if(pid==0){
		// copy torrent to destination
		execl("/bin/cp", "/bin/cp", torrentPath, destination, (char *)0);
	}else{
		wait(0);
		strcpy(infoHash, getInfoHash(destination));
		track(infoHash, trackers);
		// ????
	}
	return 1;
}