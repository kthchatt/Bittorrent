#ifndef _swarm_h
#define _swarm_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//max swarms should be equal to max torrent
#define SIGNATURE "-XX0000-"
#define MAX_SWARMS 4
#define MAX_SWARM_SIZE 200
#define MAX_TRACKERS 4
#define MAX_URL_LEN 100
#define boolean int
#define true 1
#define false 0

//swarms contain all swarm-connected peers, built from tracker queries.
//before every scrape, clear scrape data and repopulate.
 typedef struct
 {
 	int taken;
 	char* tracker 	[MAX_TRACKERS];
 	char* ip  		[MAX_SWARM_SIZE];
 	char* port		[MAX_SWARM_SIZE];
 	char  peer_id   [20];
 	char  info_hash [20];
 	int listenport, scrape_completed, scrape_incomplete;
 	//scrape data
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

#endif