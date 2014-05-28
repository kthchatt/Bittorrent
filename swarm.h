/* swarm.c
 * 2014-04-24
 * Robin Duda
 *  Swarming.
 */

#ifndef _swarm_h
#define _swarm_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "peerwire.h"
#include "bencodning.h"
#include "protocol_meta.h"


int swarm_select(torrent_info* tinfo);	//return a free swarm
void swarm_lock(int index);				//lock the mutex of swarm index
void swarm_unlock(int index);			//unlock the mutex of swarm index
void swarm_reset(swarm_t* swarm);		//clear the swarm, before announcing to get rid of stale peers.
void swarm_listen(swarm_t* swarm);		//sets the swarm to listen mode
void swarm_release(swarm_t* swarm);		//release the swarm, tracker stopped or failure.
void swarm_scour(swarm_t* swarm);		//connect to peers in the swarm
int swarm_peercount(int swarm_id);		//get amount of peers in swarm.
void swarm_initialize(); 				//initialize swarms.
int swarm_incomplete(int swarm_id);		//returns the amount of leechers
int swarm_completed(int swarm_id);		//returns the amount of seeders

#endif