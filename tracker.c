/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "announce.h"
#include "scrape.h" 

#define SIGNATURE "-XX0000-"

 //todo: save a peerlist with ip:port and info_hash.
 //todo: threaded announce and scrape.
 //todo add listener.

char* peer_id;	//allocate for peer ID.

void generate_id()
{
	int i, len;

	peer_id = (char*) malloc(21);
	strcpy(peer_id, SIGNATURE);
	len = strlen(SIGNATURE);

	for (i = len; i < 20; i++)
		if (rand()%2 == 0)
			peer_id[i] = (char) (rand()%9+48);	//generate 0..9
		else
			peer_id[i] = (char) (rand()%25+65); //generate A-Z
}

int main(int argc, char ** argv)
{
	generate_id();

	tracker_scrape("http://127.0.0.1/tracker/announce", "INFO_HASH_0000000000"); //include struct ptr to save scrape data
	
	tracker_announce("http://127.0.0.1/tracker/announce.php", "INFO_HASH_0000000000", 
					   peer_id, "10.0.0.0", "Completed", 123918, 123918);

}
