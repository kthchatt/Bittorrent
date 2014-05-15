#ifndef _tracker_h
#define _tracker_h

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "announce.h"
#include "scrape.h" 
#include "swarm.h"
#include "peerwire.h"
#include "netstat.h"
#include "bencodning.h"
#include "protocol_meta.h"

//adds a info_hash to be tracked, specify trackers in char* name[MAX_TRACKERS] format.
int tracker_track(torrent_info* tinfo);
//stops tracking of info_hash, freeing memory, swarm slot and notifies the tracker.
void tracker_untrack(torrent_info* tinfo);


//add methods for retrieveing scrape, peerlist etc.. for GUI display only.
#endif 