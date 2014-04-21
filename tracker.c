#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "announce.h"

 //todo: save a peerlist with ip:port and info_hash.

int main(int argc, char ** argv)
{

//int tracker_announce(char* tracker, char* info_hash, char* peer_id, char* ip, char* event, int downloaded, int left, peerlist);
	tracker_announce("http://127.0.0.1:80/tracker/announce.php", "INFO_HASH_0000000000", "PEER_ID_000000000002", "10.0.0.0", "Started", 0, 123918);
}
