//gcc main.c init_torrent.c bencodning.c createfile.c -o main.out -pthread -lssl -lcrypto
//TODO !!!! Add create file to decode bencode.

#define MAX_TORRENTS 100
#define MAX_TRACKERS 10
#include "init_torrent.h"
#include "writepiece.h"


torrent_info *torrents[MAX_TORRENTS];

int main (int argc, char *argv[]){

	int i;
	torrents[0] = malloc(sizeof(torrent_info));

	/*char *trackers[MAX_TRACKERS];
	for (i = 0; i < MAX_TRACKERS; i++){
		trackers[i] = torrents[0]->_announce_list[i];
	}*/


	init_torrent(argv[1], torrents[0]);
	for (i = 0; i < 4; i++){
		fprintf(stderr, "_announce_list: %s\n", torrents[0]->_announce_list[i]);
	}
	
	/*void *ptr = malloc(torrents[0]->_piece_length);
	memset(ptr, 'A', torrents[0]->_piece_length);
	write_piece(torrents[0], ptr);
	sleep(20);*/
	fprintf(stderr, "Quiting\n");
	return 1;
} 