/* swarm.c
 * 2014-04-24
 * Robin Duda
 *  Swarming.
 */

#include "swarm.h"

 static bool initialized = false;

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

void swarm_initialize()
{
	printf("\nInitializing swarm."); fflush(stdout);

	if (initialized == true)
		return;

	memset(&swarm, 0, sizeof(swarm_t) * MAX_SWARMS);
	printf("\nInitialized swarms."); fflush(stdout);

	initialized = true;
}

int swarm_incomplete(int swarm_id)
{
	if (swarm_id >= MAX_SWARM_SIZE || swarm_id < 0)
		return 0;
	
	return swarm[swarm_id].incomplete;
}

int swarm_completed(int swarm_id)
{
	if (swarm_id >= MAX_SWARM_SIZE || swarm_id < 0)
		return 0;
	
	return swarm[swarm_id].completed;
}

int swarm_peercount(int swarm_id)
{
	if (swarm_id >= MAX_SWARM_SIZE || swarm_id < 0)
		return 0;
	
	return swarm[swarm_id].peercount;
}

//return a free swarm
int swarm_select(torrent_info* tinfo)
{
	int  swarm_id = -1, i, j;

	for (i = 0; i < MAX_SWARMS; i++)
	{
		if (swarm[i].taken == false)
		{
			swarm_id = i;
			swarm[i].taken = true;
			swarm[i].peercount = 0;
			swarm[i].completed = 0;
			swarm[i].incomplete = 0;

			generate_id(swarm[i].peer_id);
			swarm[i].info_hash = tinfo->_info_hash;
			swarm[i].tinfo = tinfo;
			swarm[i].bitfield = malloc(((tinfo->_piece_length / 20) / 8) + 1);

			for (j = 0; j < MAX_TRACKERS; j++)
			{
				swarm[i].tracker[j].url = tinfo->_announce_list[j];
				swarm[i].tracker[j].alive = true;
			}

			break;
		}
	}
  	return swarm_id;
}

//clear stale peers.
void swarm_reset(swarm_t* swarm)
{
	int i;

	for (i = 0; i < MAX_SWARM_SIZE; i++)
	{
		//set sockfd to 0 !!
		/*if (swarm->peer[i].sockfd != 0)
			close(&swarm->peer[i].sockfd);
		swarm->peer[i].sockfd = 0;*/

		//memset(swarm->peer[i].ip, '\0', 21);
		//memset(swarm->peer[i].port, '\0', 6); 
	}

	swarm->peercount = 0;
}

//clear swarm peers.
void swarm_release(swarm_t* swarm)
{
	int i;

	for (i = 0; i < MAX_SWARM_SIZE; i++)
	{
		memset(swarm->peer[i].ip, '\0', 21);
		memset(swarm->peer[i].port, '\0', 6); 
	}

	swarm->peercount = 0;	
	if (swarm->sockfd != 0)
		close(swarm->sockfd);	
	swarm->taken = 0;	
}

//find a port and listen to it. 
//for every connection create a new thread. with &peer
void* peerlisten(void* arg)
{
	swarm_t* swarm = (swarm_t*) arg;

	int    sock, new_sock;    
    struct sockaddr_in addr;        
    struct sockaddr_in their_addr;     
    unsigned int    sin_size;
 
    addr.sin_family      = AF_INET;        
    addr.sin_port        = htons(0);   
    addr.sin_addr.s_addr = INADDR_ANY;      
    memset(&(addr.sin_zero), 0, 8);        
    sin_size = sizeof(addr); 
 
 	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) > -1)
 	{
 		//swarm failed to bind
 		if ((bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr))) < 0)
    		return arg;
    	else
    		if ((listen(sock, BACKLOG)) < 0)
				return arg;
 	} 
 	else
 		 return arg;

    getsockname(sock, (struct sockaddr *)&addr, &sin_size);
    swarm->listenport = addr.sin_port;
	printf("Listening for peers, port = %d, %d", swarm->listenport, addr.sin_port);

	while (swarm->taken == true)
	{
		sin_size = sizeof(struct sockaddr_in);
        new_sock = accept(sock, (struct sockaddr *)&their_addr, &sin_size);
        printf("\n------------ INCOMING CONNECTION ON: %d --------------------", new_sock);
        //on accept send sockfd to peer.
        //spawn peerwire thread.
            /*swarm->peer[i].info_hash = swarm->info_hash;
    		swarm->peer[i].peer_id = swarm->peer_id;
    		swarm->peer[i].tinfo = swarm->tinfo;	//peers needs access to torrent_info for reading/writing to file.*/
		//swarm->peercount++;
		sleep(1);
	}		

	return arg;
}

//find new peers and create pwp-thread.
void swarm_scour(swarm_t* swarm)
{
    int i;

    for (i = 0; i < swarm->peercount; i++)
    {

    	printf("\nPeer [#%d/%d] in Swarm: %s:%s", i, swarm->peercount, swarm->peer[i].ip, swarm->peer[i].port);

    	if (swarm->peer[i].sockfd == 0)
    	{
    		swarm->peer[i].info_hash = swarm->info_hash;
    		swarm->peer[i].peer_id = swarm->peer_id;
    		swarm->peer[i].tinfo = swarm->tinfo;	//peers needs access to torrent_info for reading/writing to file.

    		if ((pthread_create(&swarm->peer[i].thread, NULL, peerwire_thread_tcp, &swarm->peer[i])))
    			printf("\nStarting peerwire_thread_tcp.. Failed.");
    	}
    }
}

void swarm_listen(swarm_t* swarm)
{
	if(!(pthread_create(&swarm->thread, NULL, peerlisten, swarm)))
			printf("\nAdding swarm listener..");
}