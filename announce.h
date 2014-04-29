#ifndef _announce_h
#define _announce_h

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
 #include "urlparse.h" 
 #include "swarm.h"

//example call to tracker "announce".
/*announce("protocol://retracker.hq.ertelecom.ru:port/announce-url",	//URL TO TRACKER
	"iiiinnnnffffoooohashkkkkk", 										//METAINFO HASH
	"peerIDaaaabbbbcdeeff", 											//PEERID
	"0", 																//IP
	"Started",															//EVENT [started, stopped, completed] 
	0, 																	//DOWNLOADED
	120582);															//LEFT
*/

//TODO: announce must return a parsed peerlist.
//int tracker_announce(char* tracker, char* info_hash, char* peer_id, char* ip, char* event, int downloaded, int left);
int tracker_announce(swarm_t* swarm);

#endif