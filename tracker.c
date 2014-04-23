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

swarm_t swarm[MAX_SWARMS];
pthread_t torrents[MAX_SWARMS];

void debug(int postal) 
{ 
    printf("\n__point_%d_exec__\n", postal); 
    fflush(stdout); 
}

//generates 20bytes long swarm-unique peer identifier. (one id per swarm)
void generate_id(char* peer_id)
{
	int i, len;

	strcpy(peer_id, SIGNATURE);
	len = strlen(SIGNATURE);

	for (i = len; i < 20; i++)
		if (rand()%2 == 0)
			peer_id[i] = (char) (rand()%9+48);	//generate 0..9
		else
			peer_id[i] = (char) (rand()%25+65); //generate A-Z

		printf("\n%s\n", peer_id);
}

static void* tracking(void* arg)
{
	int i;
	swarm_t *swarm = (swarm_t*) arg;

	srand(time(NULL));

	while (swarm->taken == true)
	{
		sleep(rand()%5+3);
		
		for (i = 0; i < MAX_TRACKERS; i++)
		{
		//swarm needs to be URL_ENCODED
			if (strlen(swarm->tracker[i]) > 0)
			{
				//tracker_scrape(swarm->tracker[i], swarm->info_hash);
				tracker_announce(swarm->tracker[i], swarm->info_hash, swarm->peer_id, "10.0.0.0", "started", 8016, 123918);
			}
		}
	}
}

void track(char* info_hash, char* trackers[MAX_TRACKERS])
{
	int i, j;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].taken == false)
		{
			swarm[i].taken = true;
			swarm[i].peer_id = (char*) malloc(21);
			generate_id(swarm[i].peer_id);
			strcpy(swarm[i].info_hash, info_hash);

			for (j = 0; j < MAX_TRACKERS; j++)
			{
				swarm[i].tracker[j] = (char*) malloc(MAX_URL_LEN);
				strcpy(swarm[i].tracker[j], trackers[j]);
			}

			if(!(pthread_create(&torrents[i], NULL, tracking, &swarm[i])))
				printf("\nTracking: %s as [%s]", swarm[i].info_hash, swarm[i].peer_id);

			return;
		}
	}
	printf("Swarms are busy! Please increase MAX_SWARMS or fix a memory leak!");
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
										"http://127.0.0.1:80/tracker/announce.php"};;

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