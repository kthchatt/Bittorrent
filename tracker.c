/* tracker.c
 * 2014-04-22
 * Robin Duda
 *  Peer Tracker.
 */

 //CALL INIT WHEN INCLUDING THIS FILE.

 //todo: add listen.c for peerwire, 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "announce.h"
#include "scrape.h" 

//max swarms should not be lesser than max torrents.
#define SIGNATURE "-XX0000-"
#define MAX_SWARMS 5
#define MAX_SWARM_SIZE 200
#define MAX_TRACKERS 4
#define MAX_URL_LEN  100
#define boolean int
#define true 1
#define false 0

//swarms contain all swarm-connected peers, built from tracker queries.
 typedef struct
 {
 	boolean taken;
 	char* tracker 	[MAX_TRACKERS];
 	char* ip  		[MAX_SWARM_SIZE];
 	char* port		[MAX_SWARM_SIZE];
 	char* peer_id;
 	char  info_hash [20];
 	int listenport;
 	pthread_mutex_t lock;
 } swarm_t;


swarm_t swarm[MAX_SWARMS];
pthread_t torrents[MAX_SWARMS];

void debug(int postal) 
{ 
    printf("\n__point_%d_exec__\n", postal); 
    fflush(stdout); 
}

//generates 20bytes long swarm-unique peer identifier.
//one id per swarm should be generated.
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
				tracker_scrape(swarm->tracker[i], swarm->info_hash);
				tracker_announce(swarm->tracker[i], swarm->info_hash, swarm->peer_id, "10.0.0.0", "started", 8016, 123918);
			}
		}
	}
}

//add a torrent to track.
//info_hash, tracker urls
//one thread per torrent, swarm is global.
//find a free swarm allocation
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

			//copy trackers
			for (j = 0; j < MAX_TRACKERS; j++)
			{
				swarm[i].tracker[j] = (char*) malloc(MAX_URL_LEN);
				strcpy(swarm[i].tracker[j], trackers[j]);
			}

			if(!(pthread_create(&torrents[i], NULL, tracking, &swarm[i])))
			{
				printf("\nTracking: %s", swarm[i].info_hash);
			}

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

			// FREE/CLEAR IP
			// FREE/CLEAR PORT
			// FREE THREAD.

			swarm[i].taken = false;
		}
	}
}

//initialize.
void init()
{

	//generate_id();
	//swarm_init();
	//listen();
	//spawn a thread for every torrent, include all tracker urls. 
	//thread should maintain the swarm, and listen for new peers. "peerwire.c"
}

int main(int argc, char ** argv)
{
		/*generate_id();
		tracker_scrape("http://127.0.0.1/tracker/announce.php", "INFOHASHAAAAAAAAAAAA"); //include struct ptr to save scrape data
		tracker_announce("http://127.0.0.1/tracker/announce.php", "INFOHASHAAAAAAAAAAAA", 
					   peer_id, "10.0.0.0", "completed", 8016, 123918);*/
		char *trackers[MAX_TRACKERS] = {"", 
										"", 
										"", 
										"http://127.0.0.1:80/tracker/announce.php"};;

		track("00000000000000000001", trackers); usleep(1000);
		track("00000000000000000001", trackers);usleep(1000);
		track("00000000000000000001", trackers);usleep(1000);
		track("00000000000000000001", trackers);usleep(1000);
		track("00000000000000000001", trackers);usleep(1000);
		track("00000000000000000001", trackers);usleep(1000);

		while (1)
		{
			usleep(50000);
			printf("!");
			fflush(stdout);
		}
}