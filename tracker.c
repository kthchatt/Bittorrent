/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

 //todo: add listen.c for peerwire.

#include "tracker.h"

pthread_t torrents[MAX_SWARMS];

static void* tracking(void* arg)
{
	int i;
	swarm_t* swarm = (swarm_t*) arg;

	srand(time(NULL));

	while (swarm->taken == true)
	{
		usleep(50000);
		tracker_scrape(swarm);
		tracker_announce(swarm);				//completed/stopped events are to be sent at a later stage.

		//download, upload, messaging. main torrent loop.

		printf("Peercount: %d\n", swarm->peercount);
	}
}

void track(char* info_hash, char* trackers[MAX_TRACKERS])
{
	int swarm_id;

	if ((swarm_id = swarm_select(info_hash, trackers)) > -1)
	{
		if(!(pthread_create(&torrents[swarm_id], NULL, tracking, &swarm[swarm_id])))
			printf("\nTracking: %s as [%s]\n", swarm[swarm_id].info_hash, swarm[swarm_id].peer_id);
	}
	else printf("\nSwarms are busy! Increase MAX_SWARMS or fix a memory leak!\n");

	return;
}

void untrack(char* info_hash)
{
	int i, j;

	for (i = 0; j < MAX_SWARMS; i++)
	{
		if (strcmp(swarm[i].info_hash, info_hash) == 0)
		{
			//notify tracker of event=stopped.
			//call swarm_free/swarm_release
			swarm[i].taken = false;
		}
	}
}


//memory leak confirmed.
int main(int argc, char ** argv)
{
		char *trackers[MAX_TRACKERS] = {"http://127.0.0.1:80/tracker/announce.php", 
										"", 
										"", 
										""};

		track("00000000000000000001", trackers);
		track("00000000000000000001", trackers);
		track("00000000000000000001", trackers);

		while (1)
		{
			usleep(50000);
			printf("!");
			fflush(stdout);
		}
}