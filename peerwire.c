/* peerwire.c
 * 2014-05-06
 * Robin Duda
 *  Peerwiring.
 */

#include "peerwire.h"


 //todo: keep track of pieces in flight.

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
//<len=0001+X><id=5><bitfield>
void bitfield(peer_t* peer)
{
	/*int piece_count, i;						//piece_count from include.
	int payload = 0, len;
    char* bitfield;
    unsigned char id = 5; 	

    //piece_count = get_bitfield(torrent_info_pointer, bitfield);	
    len = htons(piece_count + 1);

    char* request = malloc(1 + piece_count/8+1);
    memcpy(request, &len, 4);					payload += 4;
    memcpy(request + payload, &id, 1);			payload += 1;
    memcpy(request + payload, bitfield, piece_count); 	payload += piece_count/8+1;

    send(peer->sockfd, request, payload, 0);		
    free(request);	*/
}

//message [choke, unchoke, interested, not interested]
void message(peer_t* peer, unsigned char message)
{
	int payload = 0, len = htonl(1);
    char* request = malloc(1 + 1);

    switch (message)
    {
    	case CHOKE:   peer->choking = true; 			break;
    	case UNCHOKE: peer->choking = false; 			break;
    	case INTERESTED: peer->interested = true; 		break;
    	case NOT_INTERESTED: peer->interested = false; 	break;
    }

    memcpy(request, &len, 4);						payload += 4;
    memcpy(request + payload, &message, 1);			payload += 1;
    send(peer->sockfd, request, payload, 0);		
    free(request);
}

//call when a piece has been successfully downloaded.
void have(peer_t* peer, int piece_index)
{
	int payload = 0, len = htonl(5);
	unsigned char id = 4;
	char* request = malloc(4 + 1 + 4);

	memcpy(request, &len, 4); 						payload += 4;
	memcpy(request + payload, &id, 1);				payload += 1;
	memcpy(request + payload, &piece_index, 4); 	payload += 4;
	send(peer->sockfd, request, payload, 0);	
	free(request);
}

//while .. read.. dgram
void* listener_udp(peer_t* peer)
{
	return peer;
}

//todo not yet implemented
void* peerwire_thread_udp(peer_t* peer)
{
	//pthread_create(&thread, null, listener_udp, peer);
	while (peer->sockfd != 0)
		sleep(1);

	return peer;
}

//hex formatting.
void printf_hexit(char* buf, int num)
{
	int i, a;

	for (i = 0; i < num; i++)
	{
		if (i > 0)
		{
			if ((i%8 == 0))
				printf("\t");

			if ((i%16 == 0))
			{
			/*	printf("\t");
				for (a = 0; a < 15 && i+a < num; a++)
				{
					printf("%c", buf[i+a]);
				}*/
				printf("\n");
			}
		}
		printf("%02x  ", (unsigned char) buf[i]);
	}
	printf("\n------------------------------------------------------------------------");
}

void debug(int postal)
{
	printf("\n__debug_%d__", postal);
	fflush(stdout);
}

//todo: fill one piece from the buffer, return num with the offset.
void receive_piece(char* buffer, int* num, int* msglen, peer_t* peer)
{
	int downloaded = 0, left = *msglen - 9, length = *msglen - 9, index, offset, header = 13;
	char* block = malloc(length);

	if (block == NULL)
	{
		*num = 0;
		printf("\nCritical! Buffer is choked on memory! [Out of RAM]");
		return;
	}

	//printf("\nThis. \\%02x\\%02x\\%02x\\%02x\\%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]); fflush(stdout);
	//printf("\nNumbytes: %d, Msglen: %d\n", *num, *msglen); fflush(stdout);
	//printf_hexit(buffer, *msglen + 4);
	memset(block, 0, 16384);
	memcpy(&index, buffer + 8, 4);
	memcpy(&offset, buffer + 5, 4);

	printf("\n(RST) Downloading new Piece #%d, at Offset %d.", index, offset);

	while (downloaded < 16384)
	{
		if (left > *num + header)
		{
			memcpy(block + downloaded, buffer + header, *num); //2ndarg + header
			left -= (*num -header); //+header
			downloaded = length - left;
			printf("\n(Downloading) Downloading Piece %d/%d \tIndex: %d\tOffset: %d\tPayload: %d\t", downloaded, length, index, offset, *num + header); fflush(stdout);
			/*if ((*/*num = recv(peer->sockfd, buffer, left, 0);/*) == 0)

				return;*/ 
		}
		else
			if (left <= *num - header) //removed +header from num
			{
				*num -= (left + header);
				memcpy(block + downloaded, buffer + header, left);
				//printf_hexit(block, 16384);
				//printf("\nLeft to download %d, (bytes to shift buffer.)", *num);
				memmove(buffer, buffer + left + header, *num);
				//if (*num > 0)
				//	printf("\nNext. \\%02x\\%02x\\%02x\\%02x\\%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]); fflush(stdout);
				//send piece to fileman.---
				downloaded = length;
				netstat_update(INPUT, downloaded, peer->info_hash);
				printf("\n(Complete) Downloading Piece %d/%d \tIndex: %d\tOffset: %d\tPayload: %d\tOverflow: %d\tBF check: [%2x, %2x]", downloaded, length, index, offset, *num + left, *num, block[0], block[16383]); fflush(stdout);
			}
		header = 0;
	}
}

//every time data is received update lastrecv with system tick.
//listens to incoming data/messages
//todo: if not interesting and not interested drop the connection and mark peer as such. after 5 minutes reconnect..
void* listener_tcp(void* arg)
{
	peer_t* peer = (peer_t*) arg;
	char recvbuf[DOWNLOAD_BUFFER];
	char* message = (char*) malloc(45);
	int num = 0, msglen;

	printf("\n[sockfd = %d]\tTCP Listener init.", peer->sockfd);

	while (peer->sockfd != 0)
	{
		if (num == 0)
		  memset(recvbuf, 0, DOWNLOAD_BUFFER); //for debug only
		memset(message, '\0', 45);

		if (num > 0 || (num = recv(peer->sockfd, recvbuf, DOWNLOAD_BUFFER, 0)) > 0)
		{
			memcpy(&msglen, recvbuf, 4);
			//printf("\nmsglen: %d", msglen);
			msglen = htonl(msglen);
			//printf("\nmsglen: %d", msglen);	

			if (msglen > 0)
			switch ((unsigned char) recvbuf[4])
			{
				case UNCHOKE: 		peer->choked = false;    strcat(message, "UNCHOKE");			break;
				case CHOKE:			peer->choked = true;     strcat(message, "CHOKE"); 				break;
				case INTERESTED: 	peer->interested = true; strcat(message, "INTERESTED");			break;
				case NOT_INTERESTED:peer->interested = false;strcat(message, "NOT_INTERESTED");  	break;
				case HAVE: 			strcat(message, "HAVE");			break; 
				case REQUEST: 		strcat(message, "REQUEST");			break;	
				case PIECE: 		receive_piece(recvbuf, &num, &msglen, peer); 		continue;	 //todo allocate piece buffer with piece length, if alreadyexists throw cancel.
				case 84: 			strcat(message, "HANDSHAKE");		break; //Bittorrent Protocol...
				default: 			strcat(message, "UNDEFINED"); 		break;
			}
			if (num > 500)
			{
				printf("\nWarning! buffer [%d Bytes] recovered from corruption. ID {%d} [reduce workload]\n", num, recvbuf[4]);
				printf_hexit(recvbuf, num);
				sleep(10);
			}
			num = 0;
		} 
	}
	free(message);
	return arg;
}

//connect and get sockfd (if sockfd == 0)
//this thread may be invoked from the listener, where the sockfd is already set.
void* peerwire_thread_tcp(void* arg)
{
    struct addrinfo hints, *res;
    pthread_t listen_thread;
    peer_t* peer = (peer_t*) arg;

    //set up connection specific flags
    peer->choking = 1;
    peer->choked = 1;
    peer->interested = 0;
    peer->interesting = 0;

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
				return arg; //thread_exit, peer_free/peer_stale
			}
	}

	if (!(pthread_create(&listen_thread, NULL, listener_tcp, peer)))
		printf("\n[sockfd = %d]\tStarting peer listener..", peer->sockfd);

	printf("\n[sockfd = %d]\tConnected! [%s:%s], sending handshake..\n", peer->sockfd, peer->ip, peer->port); fflush(stdout);
	handshake(peer, peer->info_hash, peer->peer_id);
	//sleep(2);
	//bitfield(peer);
	//sleep(5);
	sleep(2);
	message(peer, INTERESTED);
	sleep(2);
	message(peer, UNCHOKE);
	printf("\n[sockfd = %d]\tHandshake sent.", peer->sockfd);

		int jk;
	while (peer->sockfd != 0)
	{
		//do peerstuff. //choke, unchoke, interested, not nterested, have, piece

		//check for unretrieved pieces, lock the piece, download the piece, on download complete flag as completed in swarm
		//all peers will receive HAVE message, bitfield is updated for new connecting peers.
		//when retrieving a piece download 16384 at a time, keep track of how long the piece is, so we know how much data to ask for in the final
		for (jk = 0; jk < 17; jk++)
		{
			request(peer, htonl(jk), htonl(0), htonl(16384));  //request 
					sleep(8);//usleep(500000);
		}
		//printf("\n[sockfd = %d]\tdoing peerstuff. [choked = %d, choking = %d] ^^ ", peer->sockfd, peer->choking, peer->choked);
	}
	printf("\n[sockfd = %d]\tPeer disconnected.", peer->sockfd);
	//shutdown the listener, free the peer, close the sockfd.

	return arg;
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
