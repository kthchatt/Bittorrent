#ifndef _tracker_h
#define _tracker_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//max swarms should be equal to max torrents.
const char* SIGNATURE = "-XX0000-";
const int MAX_SWARMS = 4;
const int MAX_SWARM_SIZE = 200;
const int MAX_TRACKERS = 4;
const int MAX_URL_LEN = 100;
const int true = 1;
const int false = 1;

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