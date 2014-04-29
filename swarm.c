/* swarm.c
 * 2014-04-24
 * Robin Duda
 *  Swarming.
 */

#include "swarm.h"

//all swarm methods need be thread-safe.

//generates 20bytes long swarm-unique peer identifier. (one id per swarm)
void generate_id(char peer_id[21])
{
	int i, len;

	strcpy(peer_id, SIGNATURE);
	len = strlen(SIGNATURE);

	for (i = len; i < 20; i++)
		if (rand()%2 == 0)
			peer_id[i] = (char) (rand()%9+48);	//generate 0..9
		else
			peer_id[i] = (char) (rand()%25+65); //generate A-Z

		peer_id[20] = '\0';
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
			swarm[i].peercount = 0;

			generate_id(swarm[i].peer_id);
			strcpy(swarm[i].info_hash, info_hash);

			for (j = 0; j < MAX_TRACKERS; j++)
			{
				memset(swarm[i].tracker[j].url, '\0', sizeof(MAX_URL_LEN));
				strcpy(swarm[i].tracker[j].url, trackers[j]);
			}
			break;
		}
	}
  	return swarm_id;
}

//clear swarm peers.
void swarm_reset(swarm_t* swarm)
{
	int i;

	for (i = 0; i < MAX_SWARM_SIZE; i++)
	{
		memset(swarm->ip[i], '\0', 21);
		memset(swarm->port[i], '\0', 6); 
		swarm->peercount = 0;
	}
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
