 /*  protocol_meta.h
 *	Author: Robin Duda
 *
 *	Defines the protocol & its settings.
 */

 #ifndef _protocol_meta_h
 #define _protocol_meta_h

#include <pthread.h>
#include "bencodning.h"

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
#define MAX_SWARM_SIZE 1024
#define MAX_URL_LEN 256
#define MAX_TRACKERS 16     //bencodning.c does not respect MAX_TRACKERS.
#define SCRAPE_TIME 2
#define ANNOUNCE_TIME 2
#define UDP_CONNECTION_ID  0x8019102717040000

//peerwire
#define PROTOCOL    "BitTorrent protocol"
#define CHOKE 		0
#define UNCHOKE 	1
#define INTERESTED 	2
#define NOT_INTERESTED 3
#define HAVE 		4
#define BITFIELD	5
#define REQUEST		6
#define PIECE       7
#define CANCEL      8
#define PORT        9   
#define HANDSHAKE   84
#define EXTENDED    20
#define DOWNLOAD_BUFFER 2388608
#define BLOCK_SIZE	16384
#define CHOKE_BACKOFF 30
#define PIECE_WAIT    100
#define PIECE_QUEUE   1
#define CONNECTION_LIMIT 50    

 //current implementation of peerwire requires PIECE_QUEUE to be 1 per peer,
 //combined with the required PIECE_WAIT this introduces additional latency.
 //this will be solved in the future by implementing a piece buffer that is
 //larger than one piece.

//units and rates.
#define U_NONE 0
#define U_BYTE 1
#define U_KILO 1000
#define U_MEGA 1000000
#define U_GIGA 1000000000
#define DOUBLE_PRECISION 1e-5

//sizes
#define S_BYTE " B"
#define S_KILO " KB"
#define S_MEGA " MB"
#define S_GIGA " GB"

//rates
#define R_BYTE "B/s"
#define R_KILO "KB/s"
#define R_MEGA "MB/s"
#define R_GIGA "GB/s"

#define FORMATSTRING_LEN 24

typedef struct
{
	char* url;
	boolean alive;
	int completed, incomplete, downloaded, interval, minterval;
} tracker_t;

//ing = local, ed = remote, todo: add pointer to swarm, remove peer_id & info_hash. (taken is required?)
typedef struct peer_t
{
	int sockfd;
	int choked, choking; 
	int interested, interesting;
	int queued_pieces;
	char ip[21];
	char port[6];
	char* peer_id;			//pointers to swarm_t data. (required for threading.)
	char* info_hash;
	char* bitfield_peer;	//bits the remote peer is in posession of.
	char* bitfield_sync;	//local bits that have been announced, diff this to swarm and then 'have'-, then update.
	torrent_info* tinfo;
	struct swarm_t* swarm;
	pthread_t thread;
} peer_t;

//swarms contain all swarm-connected peers, built from tracker queries.
//before every scrape, clear scrape data and repopulate.
 typedef struct swarm_t
 {
 	int taken;
 	tracker_t tracker 	[MAX_TRACKERS];
 	peer_t peer 		[MAX_SWARM_SIZE];
 	char  peer_id   	[21];
 	char* info_hash;
 	char* bitfield;						//bitfield, set bit to 1 when downloaded set bit to 1 when started download, clear bit if download failed.
 	int listenport, peercount, sockfd, completed, incomplete;
 	torrent_info* tinfo;
 	pthread_t thread;
 	pthread_mutex_t peerlock, bitlock;	//lock the peerlist, lock the bitfield.
 } swarm_t;

 typedef struct 
{
    tracker_t* tracker;
    swarm_t* swarm;
    pthread_t thread;
} scrape_t;

 typedef struct 
{
    tracker_t* tracker;
    swarm_t* swarm;
    pthread_t thread;
} announce_t;

 swarm_t swarm[MAX_SWARMS];

 #endif