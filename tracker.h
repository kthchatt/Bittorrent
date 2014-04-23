#ifndef _tracker_h
#define _tracker_h

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
 	char* peer_id;
 	char  info_hash [20];
 	int listenport;
 	//scrape data
 	pthread_mutex_t lock;
 } swarm_t;


//adds a info_hash to be tracked, specify trackers in char* name[MAX_TRACKERS] format.
void track(char* info_hash, char* trackers[MAX_TRACKERS]);

//stops tracking of info_hash, freeing memory, swarm slot and notifies the tracker.
void untrack(char* info_hash);


//add methods for retrieveing scrape, peerlist etc.. for GUI display only.
//peerwire is a descendant of tracker.c
#endif