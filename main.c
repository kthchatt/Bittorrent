//gcc main.c tracker.c announce.c bencodning.c scrape.c swarm.c peerwire.c urlparse.c -o main.out -pthread -lssl -lcrypto

//#include "tracker.h"
//#include "bencodning.h"
#include "torrent.c"

int main (int argc, char *argv[]){
	/*int i;
	torrent_info data;

	char *trackers[MAX_TRACKERS];
	for (i = 0; i < MAX_TRACKERS; i++){
		trackers[i] = data._announce_list[i];
	}

	fprintf(stderr, "This is main\n");
	decode_bencode(argv[1], &data);
	fprintf(stderr, "Tracker no:0 = %s\n", trackers[0]);
	//track(data._info_hash,trackers);
	track("00000000000000000000",trackers);
	fprintf(stderr, "Tracker is running \n");
	sleep(30);
	//untrack("00000000000000000000");
	fprintf(stderr, "Stopping\n");*/

	addTorrent(argv[1]);
	return 1;
} 