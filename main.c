//gcc main.c tracker.c announce.c bencodning.c scrape.c swarm.c peerwire.c urlparse.c createfile.c -o main.out -pthread -lssl -lcrypto

#define MAX_TORRENTS 100
#include "tracker.h"
#include "bencodning.h"
#include "torrent.h"
#include "createfile.h"


torrent_info *torrents[MAX_TORRENTS];

int main (int argc, char *argv[]){

	int i;
	torrents[0] = malloc(sizeof(torrent_info));

	char *trackers[MAX_TRACKERS];
	for (i = 0; i < MAX_TRACKERS; i++){
		trackers[i] = torrents[0]->_announce_list[i];
	}


	fprintf(stderr, "This is main\n");
	decode_bencode(argv[1], torrents[0]);
	fprintf(stderr, "Tracker no:0 = %s\n", trackers[0]);

	create_file(torrents[0]);
	/*
	track(torrents[0]->_info_hash,trackers);
	//track("00000000000000000000",trackers);
	fprintf(stderr, "Tracker is running \n");
	sleep(30);
	//untrack("00000000000000000000");
	fprintf(stderr, "Stopping\n");

	addTorrent(argv[1]);*/
	return 1;
} 