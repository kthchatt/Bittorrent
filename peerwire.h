 #ifndef _swarm_h
 #define _swarm_h

 #include <netdb.h>
 #include <unistd.h>
 #include <errno.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <arpa/inet.h> 


typedef struct
{
	int sockfd;
	int am_choking;
	int peer_choking;
	int am_interested;
	int peer_interested;
	char  ip  [20];
 	char  port[05];
 	//list pieces at peer
} peer_t;




#endif