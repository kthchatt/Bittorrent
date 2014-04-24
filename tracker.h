#ifndef _tracker_h
#define _tracker_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRACKERS 4


//adds a info_hash to be tracked, specify trackers in char* name[MAX_TRACKERS] format.
void track(char* info_hash, char* trackers[MAX_TRACKERS]);

//stops tracking of info_hash, freeing memory, swarm slot and notifies the tracker.
void untrack(char* info_hash);


//add methods for retrieveing scrape, peerlist etc.. for GUI display only.
//peerwire is a descendant of tracker.c
#endif 