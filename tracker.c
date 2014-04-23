/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

 //CALL INIT WHEN INCLUDING THIS FILE.

 //todo: add listen.c for peerwire.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "announce.h"
#include "scrape.h" 

//max swarms should not be lesser than max torrents.
#define SIGNATURE "-XX0000-"
#define MAX_SWARMS 5
#define MAX_SWARM_SIZE 200
#define MAX_TRACKERS 10
#define MAX_URL_LEN  100
#define boolean int
#define TRUE 1
#define FALSE 0

//swarms contain all swarm-connected peers, built from tracker queries.
 typedef struct
 {
 	boolean taken;
 	char* trackers 	[MAX_TRACKERS] [MAX_URL_LEN];
 	char* ip  		[MAX_SWARM_SIZE][48];
 	char* port		[MAX_SWARM_SIZE][5];
 	pthread_mutex_t lock;
 } swarm_t;

char* peer_id;
int   port;
swarm_t swarms[MAX_SWARMS];

//generates 20bytes long swarm-unique peer identifier.
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
		tracker_scrape("http://127.0.0.1/tracker/announce.php", "INFOHASHAAAAAAAAAAAA"); //include struct ptr to save scrape data
		tracker_announce("http://127.0.0.1/tracker/announce.php", "INFOHASHAAAAAAAAAAAA", 
					   peer_id, "10.0.0.0", "completed", 8016, 123918);

}

//add a torrent to track.
//info_hash, tracker urls
//one thread per torrent, swarm is global.
//find a free swarm allocation
void track(char* info_hash, char* trackers[MAX_TRACKERS][MAX_URL_LEN])
{
	//find free allocated swarm
	//configure swarm with info_hash and tracker urls
	//spawn thread with trackers and info_hash
}

void listen()
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, 0, &hints, &res);	//bind to any free port.

    // make a socket, bind it, and listen on it:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    // now accept an incoming connection:

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
}

//initialize.
void init()
{
	generate_id();
	listen();
	//spawn a thread for every torrent, include all tracker urls. 
	//thread should maintain the swarm, and listen for new peers. "peerwire.c"
}