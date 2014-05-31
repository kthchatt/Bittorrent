/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

#include "tracker.h"


pthread_t torrents[MAX_SWARMS];

static void* tracking(void* arg)
{
	swarm_t* swarm = (swarm_t*) arg;

	swarm_listen(swarm);	//set the swarm to listen for peers.
	scan_all(swarm->tinfo, (unsigned char*) swarm->bitfield);	//get bitfield.

	//[todo: keep scrape and announce threads active, let them manage their own intervals]
	while (swarm->taken == true)
	{
		if (swarm->taken == true)
			tracker_scrape(swarm);						//create thread for every scrape/announce. add timeout as fksock-thread. 
 		if (swarm->taken == true)
 			tracker_announce(swarm);					//completed/stopped events are to be sent at a later stage.
		if (swarm->taken == true)
			swarm_scour(swarm);							//find new peers and initiate connections.

		sleep(600);										//wait for 5 minutes. [todo: should not be static, should be tracker interval individually]
	}

	swarm_release(swarm);
	return arg;
}

int tracker_track(torrent_info* tinfo)
{
	int swarm_id = -1;

	netstat_track(tinfo->_info_hash, tinfo->_total_length);

	if ((swarm_id = swarm_select(tinfo)) > -1)
	{
		if (pthread_create(&torrents[swarm_id], NULL, tracking, &swarm[swarm_id]))
			printf("\nTracking as [%s] .. Failed.\n", swarm[swarm_id].peer_id);
	}

	return swarm_id;
}

void tracker_untrack(torrent_info* tinfo)
{
	int i, k;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].info_hash == tinfo->_info_hash)
		{
			swarm[i].taken = false;	

			for (k = 0; k < swarm[i].peercount; k++)
				swarm[i].peer[k].sockfd = 0;
		}
	}
}