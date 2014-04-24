/* swarm.c
 * 2014-04-24
 * Robin Duda
 *  Swarming.
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "swarm.h"

//all swarm methods need be thread-safe.

//generates 20bytes long swarm-unique peer identifier. (one id per swarm)
void generate_id(char peer_id[20])
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

//return a free swarm
int swarm_select(char* info_hash, char* trackers[MAX_TRACKERS])
{
	int  swarm_id = -1, i, j;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].taken == false)
		{
			swarm_id = i;
			swarm[i].taken = true;

			generate_id(swarm[i].peer_id);
			strcpy(swarm[i].info_hash, info_hash);

			for (j = 0; j < MAX_TRACKERS; j++)
			{
				swarm[i].tracker[j] = (char*) malloc(MAX_URL_LEN);
				strcpy(swarm[i].tracker[j], trackers[j]);
			}

			break;
		}
	}
  	return swarm_id;
}

//free resources allocated by the swarm.
void swarm_release(int index)
{
	//free?
}

//lock the mutex of swarm index
void swarm_lock(int index)
{

}

//unlock the mutex of swarm index
void swarm_unlock(int index)
{

}
