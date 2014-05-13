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

	//swarm_listen(swarm);	//set the swarm to listen for peers.

	while (swarm->taken == true)
	{
		sleep(4);									//wait for the swarm to bind.
		if (swarm->taken == true)
			tracker_scrape(swarm);						//create thread for every scrape/announce. add timeout as fksock-thread. 
 		if (swarm->taken == true)
 			tracker_announce(swarm);					//completed/stopped events are to be sent at a later stage.
		if (swarm->taken == true)
			swarm_scour(swarm);							//find new peers and initiate connections.
	}

	printf("\nError: Undefined. Releasing swarm...");
	swarm_release(swarm);

	return arg;
}

int tracker_track(char* info_hash, char* trackers[MAX_TRACKERS])
{
	int swarm_id;

	netstat_initialize();
	netstat_track(info_hash);

	if ((swarm_id = swarm_select(info_hash, trackers)) > -1)
	{
		if(!(pthread_create(&torrents[swarm_id], NULL, tracking, &swarm[swarm_id])))
			printf("\nTracking: %s as [%s]\n", swarm[swarm_id].info_hash, swarm[swarm_id].peer_id);
	}
	else printf("\nSwarms are busy! Increase MAX_SWARMS or fix a memory leak!\n");

	return swarm_id;
}

void tracker_untrack(char* info_hash)
{
	int i;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].info_hash == info_hash)
		{
			swarm[i].taken = false;	//call swarm_free. This is not thread-safe nor a reliable mean to stop swarm-threads. Peer threads are not stopped.
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
