//gcc init_torrent.c bencodning.c createfile.c -c -o inittorrent.o -pthread -lssl -lcrypto
//TODO !!!! Add create file to decode bencode.

#include "init_torrent.h"

int init_torrent (char *argv, torrent_info *torrent){
	pthread_t pthread_init_torrent;

	//fprintf(stderr, "This is main\n");
	decode_bencode(argv, torrent);
	//fprintf(stderr, "Tracker no:0 = %s\n", trackers[0]);
	pthread_create(&pthread_init_torrent, NULL, create_file,torrent);
	//create_file(torrent);
	sleep(1);
	while (create_file_status(torrent) < 1){
		usleep(200000);
		fprintf(stderr, "%f \n", create_file_status(torrent));
	}
	//fprintf(stderr, "File path: %s\n", torrents[0]->_file_path[0]);
	return 1;
} 