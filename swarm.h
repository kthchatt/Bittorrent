#ifndef _swarm_h
#define _swarm_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "peerwire.h"


//max swarms should be equal to max torrent
#define SIGNATURE "NSA-PirateBust-"
#define MAX_SWARMS 4
#define MAX_SWARM_SIZE 200
#define MAX_TRACKERS 4
#define MAX_URL_LEN 100
#define boolean int
#define true 1
#define false 0

typedef struct
{
	char url [MAX_URL_LEN];
	int scrape_completed, scrape_incomplete, scrape_downloaded, announce_interval, announce_minterval;
} tracker_t;

typedef struct 
{
	int sockfd;
	int choked, choking;
	int interested, interesting;
	char ip[21];
	char port[6];
} peer_t;

//swarms contain all swarm-connected peers, built from tracker queries.
//before every scrape, clear scrape data and repopulate.
 typedef struct
 {
 	int taken;
 	tracker_t tracker 	[MAX_TRACKERS];
 	//peer_t peer 		[MAX_SWARM_SIZE];
 	//piece				[];						//piece container, store all piece data, index, have/missing
 	char  ip  			[MAX_SWARM_SIZE][21];
 	char  port			[MAX_SWARM_SIZE][6];
 	char  peer_id   	[21];
 	char  info_hash 	[21];
 	int listenport, peercount;
 	pthread_mutex_t lock;
 } swarm_t;

 swarm_t swarm[MAX_SWARMS];

 //return a free swarm
int swarm_select(char* info_hash, char* trackers[MAX_TRACKERS]);
//free resources allocated by the swarm.
void swarm_release(int index);
//lock the mutex of swarm index
void swarm_lock(int index);
//unlock the mutex of swarm index
void swarm_unlock(int index);
//clear the swarm, before announcing to get rid of stale peers.
void swarm_reset(swarm_t* swarm);

#endif