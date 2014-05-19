 #ifndef _protocol_meta_h
 #define _protocol_meta_h

#include <pthread.h>
#include "bencodning.h"

/*  protocol_meta.h
 *	Author: Robin Duda
 *
 *	
 */

//sys-global
#define true 1
#define false 0
#define boolean char

//connection specifics
#define DOWNLOAD_QUEUE 128
#define UPLOAD_QUEUE 128
#define UPLOAD_CONCURRENT 8
#define DOWNLOAD_CONCURRENT 8
//when the concurrent is full, peer will be choked.
//when the concurrent is not full, peers will be unchoked.

//swarm-tracker
#define SIGNATURE "BT-CookieCrumb-"
#define BACKLOG 8
#define MAX_SWARMS 8
#define MAX_SWARM_SIZE 256
#define MAX_URL_LEN 250
#define MAX_TRACKERS 15
#define SCRAPE_TIME 2
#define ANNOUNCE_TIME 2

//peerwire
#define PROTOCOL    "BitTorrent protocol"
#define CHOKE 		0
#define UNCHOKE 	1
#define INTERESTED 	2
#define NOT_INTERESTED 3
#define BITFIELD	5
#define REQUEST		6
#define HAVE 		4
#define PIECE       7
#define CANCEL      8
#define PORT        9   
#define DOWNLOAD_BUFFER 2388608
#define BLOCK_SIZE	16384

//units and rates.
#define U_NONE 0
#define U_BYTE 1
#define U_KILO 1000
#define U_MEGA 1000000
#define U_GIGA 1000000000

//sizes
#define S_BYTE " B"
#define S_KILO " KB"
#define S_MEGA " MB"
#define S_GIGA " GB"

//rates
#define R_BYTE "B/s"
#define R_KILO "KB/s"
#define R_MEGA "MB/s"

#define FORMATSTRING_LEN 24

typedef struct
{
	char* url;
	boolean alive;
	int completed, incomplete, downloaded, interval, minterval;
} tracker_t;

//ing = local, ed = remote, todo: add pointer to swarm, remove peer_id & info_hash. (taken is required?)
typedef struct 
{
	int sockfd;
	int choked, choking; 
	int interested, interesting;
	char ip[21];
	char port[6];
	char* peer_id;		//pointers to swarm_t data. (required for threading.)
	char* info_hash;
	torrent_info* tinfo;
	pthread_t thread;
} peer_t;

//swarms contain all swarm-connected peers, built from tracker queries.
//before every scrape, clear scrape data and repopulate.
 typedef struct
 {
 	int taken;
 	tracker_t tracker 	[MAX_TRACKERS];
 	peer_t peer 		[MAX_SWARM_SIZE];
 	char  peer_id   	[21];
 	char* info_hash;
 	int listenport, peercount, sockfd, completed, incomplete;
 	torrent_info* tinfo;
 	pthread_t thread;
 	pthread_mutex_t peerlock;
 } swarm_t;

 swarm_t swarm[MAX_SWARMS];

 #endif