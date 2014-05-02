 //entry point in tracker threads. (functions will be threaded too.)


#include "peerwire.h"
#include "swarm.h"

/*
	PIECE INDEXES ARE ZERO-BASED
	BLOCK SPECIFIES BYTE OFFSET IN PIECE
	DROP PACKETS WITH INVALID LENGTH

	MESSAGE HEADER
		4 BYTES LENGHT SPECIFIER
		1 BYTE MESSAGE TYPE
		4 BYTE INDEX | VARIABLE SIZE PAYLOAD

	HEADER MESSAGES
		choke: 			<len=0001><id=0> 						
		unchoke: 		<len=0001><id=1>						
		interested: 	<len=0001><id=2>					
		not interested: <len=0001><id=3>				
		have: 			<len=0005><id=4><piece index>
		request:		<len=0013><id=6><piece index><begin offset><requested length, piece len?>

	PAYLOAD MESSAGES
		piece: 	<len=0009+X><id=7><index><begin><block> 
		cancel: <len=0013>  <id=8><index><begin><length>		
		port: 	<len=0003>  <id=9><listen-port>
*/

#define PROTOCOL    "BitTorrent protocol"
#define CHOKE 		0
#define UNCHOKE 	1
#define INTERESTED 	2
#define NINTERESTED 3
#define HAVE 		4
#define PIECE       7
#define CANCEL      8
#define PORT        9

	#include <openssl/sha.h>


//finds a peer in the swarm with piece_id. (todo: ignore peers who are choking client), in swarm or peerwire?
/*
peer_t* piecefinder(swarm_t* swarm, int piece_id)
{
	int i, j, peer_index;

	//for all peers in swarm
	for (i = 0; i < swarm->peercount; i++)
	{
		//for all pieces in peer
		for (j = 0; j < swarm->peer[i].piece_count; j++)
			if (swarm->peer[i].piece[j] == piece_id)
			{
				return swarm->peer[i].sockfd;
			}
	}

	return NULL;
}
*/

void handshake(peer_t* peer, char info_hash[20], char peer_id[20])
{
	//handshake
	int payload = 0;
    struct addrinfo hints, *res;
    unsigned char protocol_len = strlen(PROTOCOL);
    unsigned char reserved[8];
    char* request = malloc(1 + protocol_len + 8 + 20 + 20);

    memset(reserved, 0, 8);
    reserved[5] = 16;
    reserved[7]  = 5;

    sprintf(request, "%c%s", protocol_len, PROTOCOL);	payload += 1 + protocol_len;
    memcpy(request + payload, reserved, 8);				payload += 8;
    memcpy(request + payload, info_hash, 20);			payload += 20;
    memcpy(request + payload, peer_id, 20);				payload += 20;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(peer->ip, peer->port, &hints, &res);

    //if sock open, close first.
    if (peer->sockfd != 0)
    	close(peer->sockfd);

    if ((peer->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
    {
        if (connect(peer->sockfd, res->ai_addr, res->ai_addrlen) > -1)
        {
            send(peer->sockfd, request, payload, 0);	//strlen will find the reserved byte.
            //receive?
        } 
    }
}


//<len=0013><id=6><piece index><begin offset><requested length, piece len?>
void request(peer_t* peer, int piece_index, int offset_begin, int offset_length)
{
	int payload = 0, len = 13;
	unsigned char id = 6;
    char* request = malloc(4 + 1 + 4 + 4 + 4);

    memcpy(request, &len, 4);						payload += 4;
    memcpy(request, &id, 1);						payload += 1;
    memcpy(request + payload, &piece_index,  4);	payload += 4;
    memcpy(request + payload, &offset_begin, 4);	payload += 4;
    memcpy(request + payload, &offset_length,4);	payload += 4;

    send(peer->sockfd, request, payload, 0);	//strlen will find the reserved byte.
}

void choke(void)
{

}

void unchoke()
{

}

void sendpiece(void)
{
	//send a piece, udp.
}

void enqueue(void)
{
	//queue requests?
}
                                                                            

void main(void)
{
	peer_t peer;
	char info_hash[20];
	char peer_id[20];

	printf("-----"); fflush(stdout);

   /* char data[] = "Hello, world!";
	size_t length = sizeof(data);
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(data, length, hash);

	printf("--%s--", hash); fflush(stdout);*/

	// gcc sha1Openssl.c -o sha1Openssl -lssl -lcrypto

	strcpy(peer.port, "51413");
	strcpy(peer.ip,   "127.0.0.1");

	//sprintf(info_hash, "%x", "d15b9f7471d78dd64f1419d630a8c48d708924dd");

	info_hash[0] = 0xd1;
	info_hash[1] = 0x5b;
	info_hash[2] = 0x9f;
	info_hash[3] = 0x74;
	info_hash[4] = 0x71;
	info_hash[5] = 0xd7;
	info_hash[6] = 0x8d;
	info_hash[7] = 0xd6;
	info_hash[8] = 0x4f;
	info_hash[9] = 0x14;
	info_hash[10] = 0x19;
	info_hash[11] = 0xd6;
	info_hash[12] = 0x30;
	info_hash[13] = 0xa8;
	info_hash[14] = 0xc4;
	info_hash[15] = 0x8d;
	info_hash[16] = 0x70;
	info_hash[17] = 0x89;
	info_hash[18] = 0x24;
	info_hash[19] = 0xdd;


	//strcpy(info_hash, hash);
	//strcpy(peer_id,   hash);

	handshake(&peer, info_hash, info_hash);
	//request(&peer, 0, 0, 1024);
}

//
