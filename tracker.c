/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

 //todo: add listen.c for peerwire.

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "announce.h"
#include "scrape.h" 
#include "tracker.h"
#include "swarm.h"

pthread_t torrents[MAX_SWARMS];

void debug(int postal) 
{ 
    printf("\n__point_%d_exec__\n", postal); 
    fflush(stdout); 
}

static void* tracking(void* arg)
{
	int i;
	swarm_t* swarm = (swarm_t*) arg;

	srand(time(NULL));

	while (swarm->taken == true)
	{
		sleep(3);
		printf("\nScrape Commencing!\n");
		tracker_scrape(swarm);
		//tracker_announce(swarm->tracker[i], swarm->info_hash, swarm->peer_id, "10.0.0.0", "started", 8016, 123918);
	}
}

void track(char* info_hash, char* trackers[MAX_TRACKERS])
{
	int swarm_id;

	if ((swarm_id = swarm_select(info_hash, trackers)) > -1)
	{
		if(!(pthread_create(&torrents[swarm_id], NULL, tracking, &swarm[swarm_id])))
			printf("\nTracking: %s as [%s]", swarm[swarm_id].info_hash, swarm[swarm_id].peer_id);
	}
	else printf("\nSwarms are busy! Increase MAX_SWARMS of fix a memory leak!\n");

	return;
}

void untrack(char* info_hash)
{
	int i, j;

	for (i = 0; j < MAX_SWARMS; i++)
	{
		if (strcmp(swarm[i].info_hash, info_hash) == 0)
		{
			//deallocate swarm.
			for (j = 0; j < MAX_TRACKERS; j++)
				free(swarm[i].tracker[j]);

			free(swarm[i].peer_id);

			// FREE: IP/PORT/THREAD
			swarm[i].taken = false;
		}
	}
}

int main(int argc, char ** argv)
{
		char *trackers[MAX_TRACKERS] = {"", 
										"", 
										"", 
										"http://94.228.192.98/announce"};;

		track("00000000000000000001", trackers);usleep(500000);
		track("00000000000000000001", trackers);usleep(500000);
		track("00000000000000000001", trackers);usleep(500000);
		track("00000000000000000001", trackers);usleep(500000);
		track("00000000000000000001", trackers);usleep(500000);

		while (1)
		{
			usleep(50000);
			printf("!");
			fflush(stdout);
		}
}