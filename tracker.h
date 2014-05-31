/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */
 
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
#include "searchpiece.h"

int tracker_track(torrent_info* tinfo);		//adds a info_hash to be tracked, specify trackers in char* name[MAX_TRACKERS] format.
void tracker_untrack(torrent_info* tinfo);	//stops tracking of info_hash, freeing memory, swarm slot and notifies the tracker.

#endif 