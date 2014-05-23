//gcc init_torrent.c bencodning.c createfile.c -c -o inittorrent.o -pthread -lssl -lcrypto
//TODO !!!! Add create file to decode bencode.

#include "init_torrent.h"

int init_torrent (char *argv, torrent_info *torrent){
	pthread_t pthread_init_torrent;

	int success = 1;

	if ((success = decode_bencode(argv, torrent)) == 1)
	{
		pthread_create(&pthread_init_torrent, NULL, create_file,torrent);
	} else 
	{
		success = -1;
	}
	
	return success;
}