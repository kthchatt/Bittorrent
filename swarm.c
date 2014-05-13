/* swarm.c
 * 2014-04-24
 * Robin Duda
 *  Swarming.
 */

#include "swarm.h"

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
			//strcpy(swarm[i].info_hash, info_hash);
			swarm[i].info_hash = info_hash;

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
		//set sockfd to 0 !!
		//close(&swarm->peer[i].sockfd);
		//swarm->peer[i].sockfd = 0;
		memset(swarm->peer[i].ip, '\0', 21);
		memset(swarm->peer[i].port, '\0', 6); 
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
	struct sockaddr_storage remote_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	int remote_sockfd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo("0.0.0.0", 0, &hints, &res);
	addr_size = sizeof(hints);

	printf("\nmaking the sockeht"); fflush(stdout);
	if ((swarm->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	{
		printf("\nCould not create listen port for swarm.");
		swarm->taken = 0;
		return arg;
	}
		printf("\nmaking the sockeht"); fflush(stdout);
	if ((bind(swarm->sockfd, res->ai_addr, res->ai_addrlen)) < 0);
	{
		printf("\nCould not bind to listen port in swarm.");
		swarm->taken = 0;
		return arg;
	}

	printf("\nmaking the sockeht"); fflush(stdout);
   //int getsockname(SOCKET sock, struct sockaddr *addr, int *addrlen);
	if (getsockname(swarm->sockfd, hints.ai_addr, &addr_size) < 0)
    {
    	swarm->taken = 0;
    	return arg;
    }
	else
	  swarm->listenport = ntohs(hints.ai_protocol);

	printf("Listening to peer on port: %d", swarm->listenport);

	listen(swarm->sockfd, BACKLOG);
	printf("\n[info_hash = %s]\tSwarm listening on.. %d\n", swarm->info_hash, swarm->listenport); fflush(stdout);

	while (swarm->taken == true)
	{
		addr_size = sizeof(remote_addr);
		remote_sockfd = accept(swarm->sockfd, (struct sockaddr*)&remote_addr, &addr_size);
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
    	if (swarm->peer[i].sockfd == 0)
    	{
    		swarm->peer[i].info_hash = swarm->info_hash;
    		swarm->peer[i].peer_id = swarm->peer_id;

    		if (!(pthread_create(&swarm->peer[i].thread, NULL, peerwire_thread_tcp, &swarm->peer[i])))
    			printf("\nStarting peerwire_thread_tcp..");
    	}
    }
}

void swarm_listen(swarm_t* swarm)
{
	if(!(pthread_create(&swarm->thread, NULL, peerlisten, swarm)))
			printf("\nAdding swarm listener..");
}