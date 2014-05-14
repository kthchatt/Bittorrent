//gcc main.c tracker.c announce.c bencodning.c scrape.c swarm.c peerwire.c urlparse.c createfile.c writepiece.c -o main.out -pthread -lssl -lcrypto

#define MAX_TORRENTS 100
#include "tracker.h"
#include "bencodning.h"
#include "torrent.h"
#include "createfile.h"
#include "writepiece.h"


torrent_info *torrents[MAX_TORRENTS];

int main (int argc, char *argv[]){

	int i;
	torrents[0] = malloc(sizeof(torrent_info));

	char *trackers[MAX_TRACKERS];
	for (i = 0; i < MAX_TRACKERS; i++){
		trackers[i] = torrents[0]->_announce_list[i];
	}


	//fprintf(stderr, "This is main\n");
	decode_bencode(argv[1], torrents[0]);
	//fprintf(stderr, "Tracker no:0 = %s\n", trackers[0]);
	//fprintf(stderr, "Piece length is %lld\n", torrents[0]->_piece_length);

	create_file(torrents[0]);
	//fprintf(stderr, "File path: %s\n", torrents[0]->_file_path[0]);

	void *ptr = malloc(torrents[0]->_piece_length);
	memset(ptr, 'A', torrents[0]->_piece_length);
	write_piece(torrents[0], ptr);
	sleep(20);
	fprintf(stderr, "Quiting\n");
	return 1;
} 