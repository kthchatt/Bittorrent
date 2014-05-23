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
	printf("before_scan_all"); fflush(stdout);
	scan_all(swarm->tinfo, swarm->bitfield);	//get bitfield.
	printf("after_scan_all");

	//test print
	int i, k;
	for (i = 0; i < swarm->tinfo->_hash_length / 20; i++)
		for (k = 0; k < 8; k++)
			printf("[%d]", (unsigned char) bitfield_get(&swarm->bitfield[i], k));

	sleep(1);				//2 seconds to bind.

	while (swarm->taken == true)
	{
		printf("\nScraping...."); fflush(stdout);
		if (swarm->taken == true)
			tracker_scrape(swarm);						//create thread for every scrape/announce. add timeout as fksock-thread. 
		printf("\nAnnouncing.... "); fflush(stdout);
 		if (swarm->taken == true)
 			tracker_announce(swarm);					//completed/stopped events are to be sent at a later stage.
 		printf("\nScouring.... "); fflush(stdout);
		if (swarm->taken == true)
			swarm_scour(swarm);							//find new peers and initiate connections.

		printf("\nWaiting...." ); fflush(stdout);
		sleep(300);										//sleep for interval, the lowest announce interval. In announce/scrape check last announce.
	}

	printf("\nReleasing swarm...[%s]", swarm->info_hash);
	swarm_release(swarm);

	return arg;
}

int tracker_track(torrent_info* tinfo)
{
	int swarm_id = -1;

	netstat_track(tinfo->_info_hash);

	if ((swarm_id = swarm_select(tinfo)) > -1)
	{
		if(!(pthread_create(&torrents[swarm_id], NULL, tracking, &swarm[swarm_id])))
			printf("\nTracking as [%s]\n", swarm[swarm_id].peer_id);
	}
	else printf("\nSwarms are busy! Increase MAX_SWARMS or fix a memory leak!\n");

	return swarm_id;
}

void tracker_untrack(torrent_info* tinfo)
{
	int i, k;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].info_hash == tinfo->_info_hash)
		{
			printf("\nGot untrack in tracker!"); fflush(stdout);

			swarm[i].taken = false;	//call swarm_free. This is not thread-safe nor a reliable mean to stop swarm-threads. Peer threads are not stopped.

			for (k = 0; k < swarm[i].peercount; k++)
				swarm[i].peer[k].sockfd = 0;
		}
	}
}


/*int main(int argc, char ** argv)
{
	char *trackers[MAX_TRACKERS] = {"http://127.0.0.1:80/tracker/announce.php", //http://mgtracker.org:2710/announce.php 
									"", //http://127.0.0.1:80/tracker/announce.php 
									"", 
									""};

	char* info_hash = (char*) malloc(21);	//for testing ;!
	info_hash[0] = 0xf4;
	info_hash[1] = 0x3e;
	info_hash[2] = 0x6d;
	info_hash[3] = 0x2b;
	info_hash[4] = 0x91;
	info_hash[5] = 0x3f;
	info_hash[6] = 0x22;
	info_hash[7] = 0xc3;
	info_hash[8] = 0xb0;
	info_hash[9] = 0x61;
	info_hash[10] = 0x25;
	info_hash[11] = 0x95;
	info_hash[12] = 0xf0;
	info_hash[13] = 0x25;
	info_hash[14] = 0xb1;
	info_hash[15] = 0x25;
	info_hash[16] = 0x2a;
	info_hash[17] = 0x99;
	info_hash[18] = 0x85;
	info_hash[19] = 0xdf;
	info_hash[20] = '\0';


	track(info_hash, trackers);


	while (1)
	{
		usleep(50000);
		printf("!");
		fflush(stdout);
	}
}*/
