#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "announce.h"
#include "scrape.h" 

 //todo: save a peerlist with ip:port and info_hash.

 //todo add tracker submodules, announce, listen/receive, scrape

int main(int argc, char ** argv)
{
	tracker_scrape("http://94.228.192.98/announce", "INFO_HASH_0000000000"); //include struct ptr to save scrape data
	
	tracker_announce("http://127.0.0.1/tracker/announce.php", "INFO_HASH_0000000000", 
					   "PEER_ID_000000000003", "10.0.0.0", "Completed", 123918, 123918);

	tracker_announce("127.0.0.1:80/tracker/announce.php", "INFO_HASH_0000000000", 
					 "PEER_ID_000000000009", "10.0.0.7", "Started", 0, 123918);
}
