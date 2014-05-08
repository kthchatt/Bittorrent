/* peerwire.c
 * 2014-05-06
 * Robin Duda
 *  Peerwiring.
 */

#include "peerwire.h"

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

void handshake(peer_t* peer, char* info_hash, char* peer_id)
{
	int payload = 0;
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

    send(peer->sockfd, request, payload, 0);	//strlen will find the reserved byte.
    free(request);
}


//<len=0013><id=6><piece index><begin offset><requested length, piece len?>
void request(peer_t* peer, int piece_index, int offset_begin, int offset_length)
{
	int payload = 0, len = htonl(13);
	unsigned char id = 6;
    char* request = malloc(4 + 1 + 4 + 4 + 4);

    memcpy(request, &len, 4);						payload += 4;
    memcpy(request + payload, &id, 1);				payload += 1;
    memcpy(request + payload, &piece_index,  4);	payload += 4;
    memcpy(request + payload, &offset_begin, 4);	payload += 4;
    memcpy(request + payload, &offset_length,4);	payload += 4;

    send(peer->sockfd, request, payload, 0);
    free(request);
}

//get available pieces, get piece_count, bitfield id=5
void bitfield(peer_t* peer)
{
	int piece_count = 154, i;						//piece_count from include.
	int payload = 0, len = htons(1 + piece_count);
    char* request = malloc(1 + 1);
    unsigned char id = 5; 		

    memcpy(request, &len, 4);					payload += 4;
    memcpy(request + payload, &id, 1);			payload += 1;

    //for bitwise append: multiply by i/8 to find byte, bit-shift with i%8 and OR 
	for (i = 0; i < piece_count; i++)
    {
    	//if has piece, append 1, if not has piece append 0
    	payload += 1;
    }

    send(peer->sockfd, request, payload, 0);		
    free(request);	
}

//message [choke, unchoke, interested, not interested]
void message(peer_t* peer, unsigned char message)
{
	int payload = 0, len = htonl(1);
    char* request = malloc(1 + 1);

    memcpy(request, &len, 4);						payload += 4;
    memcpy(request + payload, &message, 1);			payload += 1;

    send(peer->sockfd, request, payload, 0);		
    free(request);
}

//call when a piece has been successfully downloaded.
void have(peer_t* peer, int piece_index)
{
	int payload, len = htonl(5);
	unsigned char id = 4;
	char* request = malloc(4 + 1 + 4);

	//for every piece downloaded, set updateflag in swarm to peercount, decrease for every/have.
	//{
	//payload = 0;
	//memcpy(request, &len, 4); 						payload += 4;
	//memcpy(request + payload, &id, 1);				payload += 1;
	//memcpy(request + payload, &piece_index, 4); 		payload += 4;
	//send(peer->sockfd, request, payload, 0);	
	//}

	free(request);
}

void* listener_udp(peer_t* peer)
{
	//while .. read.. dgram..
}

//todo not yet implemented
void* peerwire_thread_udp(peer_t* peer)
{
	//pthread_create(&thread, null, listener_udp, peer);

	while (peer->sockfd != 0)
	{
		sleep(1);
	}
}

//every time data is received update lastrecv with system tick.
//listens to incoming data/messages
void* listener_tcp(void* arg)
{
	peer_t* peer = (peer_t*) arg;
	char recvbuf[2048] = {'\0'};
	int num, i, inputlen;

	printf("\n[sockfd = %d]\tTCP Listener init.", peer->sockfd);

	//uses ioctl for non-blocking read.
	while (peer->sockfd != 0)
	{
		memset(recvbuf, '\0', 2048);
		printf("\n[sockfd = %d]\tReading...", peer->sockfd);

		//if ((inputlen = ioctl(peer->sockfd, FIONREAD, &inputlen)) > 0)
		//{	
		if ((num = recv(peer->sockfd, recvbuf, 2048, 0)) > 0)
		{
			printf("\n[sockfd = %d]\tHex Dump %d Byte(s)\n------------------------------------\n", peer->sockfd, num);
			for (i = 0; i < num; i++)
			{
				printf("%02x  ", (unsigned char) recvbuf[i]);

				if ((i%8 == 0))
					printf("\t");

				if ((i%16 == 0))
					printf("\n");
			}
			printf("\n------------------------------------");
			//printf("read: %s", recvbuf);
		}
		//printf("\n[data in buffer =  %d]", num);
		sleep(1); //poll
	}
}

//connect and get sockfd (if sockfd == 0)
//this thread may be invoked from the listener, where the sockfd is already set.
void* peerwire_thread_tcp(void* arg)
{
    struct addrinfo hints, *res;
    pthread_t listen_thread;
    peer_t* peer = (peer_t*) arg;

	printf("\nConnecting to peer..");

	if (peer->sockfd == 0)
	{
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            getaddrinfo(peer->ip, peer->port, &hints, &res);

            if (!((peer->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1))
				printf("\nCould not set up socket.");
            if (!((connect(peer->sockfd, res->ai_addr, res->ai_addrlen) > -1)))
            {
				printf("\nCould not connect.");
				return; //thread_exit, peer_free/peer_stale
			}
	}

	if (!(pthread_create(&listen_thread, NULL, listener_tcp, peer)))
		printf("\n[sockfd = %d]\tStarting peer listener..", peer->sockfd);

	sleep(1);

	printf("\n[sockfd = %d]\tConnected! [%s:%s], sending handshake..\n", peer->sockfd, peer->ip, peer->port); fflush(stdout);

	handshake(peer, peer->info_hash, peer->peer_id);
	sleep(1);
	//bitfield(peer);
	sleep(7);
	message(peer, UNCHOKE);
	sleep(2);
	message(peer, INTERESTED);
	sleep(4);
	//have(peer, 1);
	request(peer, htonl(0), htonl(0), htonl(16384));
	printf("\n[sockfd = %d]\tHandshake sent.", peer->sockfd);
	//tell pieces have! (must be threadsafe)

	while (peer->sockfd != 0)
	{
		//do peerstuff. //choke, unchoke, interested, not nterested, have, piece
		printf("\n[sockfd = %d]\tdoing peerstuff. ^^", peer->sockfd);
		sleep(2);
	}
	printf("\n[sockfd = %d]\tPeer disconnected.", peer->sockfd);
	//pthread_exit_listener
}
                                                                         

/*
void main(void)
{
	peer_t peer;
	char info_hash[20];
	char peer_id[20];

	printf("-----"); fflush(stdout);


	// gcc sha1Openssl.c -o sha1Openssl -lssl -lcrypto

	strcpy(peer_id, "NSA-PirateBust-05Ac7");
	strcpy(peer.port, "6881");
	strcpy(peer.ip,   "192.168.0.10");

	//sprintf(info_hash, "%x", "d15b9f7471d78dd64f1419d630a8c48d708924dd");

	info_hash[0] = 0xf4;
	info_hash[1] = 0x3e;
	info_hash[2] = 0x6d;
	info_hash[3] = 0x2b;
	info_hash[4] = 0x91;
	info_hash[5] = 0x3f;
	info_hash[6] = 0x22;
	info_hash[7] = 0xc3;
	info_hash[8] = 0xb0;
	info_hash[9] = 0x61;
	info_hash[10] = 0x25;
	info_hash[11] = 0x95;
	info_hash[12] = 0xf0;
	info_hash[13] = 0x25;
	info_hash[14] = 0xb1;
	info_hash[15] = 0x25;
	info_hash[16] = 0x2a;
	info_hash[17] = 0x99;
	info_hash[18] = 0x85;
	info_hash[19] = 0xdf;


	//strcpy(info_hash, hash);
	//strcpy(peer_id,   hash);

	//OOO
	//handshake -> extended/bitfield/have -> interested/not interested -> unchoke/choke -> request/piece(reply)


	//piece transfer over TCP.
	handshake(&peer, info_hash, peer_id);
	sleep(1);
	message(&peer, INTERESTED);
	sleep(6);
	request(&peer, htonl(0), htonl(0), htonl(16384));
	//message(&peer, CHOKE);
	//message(&peer, NOT_INTERESTED);
	//message(&peer, UNCHOKE);
	//message(&peer, INTERESTED);
	sleep(30);
}
*/
