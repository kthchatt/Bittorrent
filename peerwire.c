/* peerwire.c
 * 2014-05-06
 * Robin Duda
 *  Peerwiring.
 */

#include "peerwire.h"
 /*
	todo:
		swarm keeps track of the actual bitfield
		peers keep track of the last informed bitfield (XOR), broadcast changes with have message, then update the local bitfield.
		keep track of the remote peers pieces with a bitfield, update on have.
		invert local bitfield and AND with remote peer to find pieces to download,
		when downloading a piece set flag in swarm to completed during download.
		in swarm: protect bitfield by lock.
 */


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
		cancel: <len=0013><id=8><index><begin><length>		
		port: 	<len=0003><id=9><listen-port>
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
	int piece_count = peer->tinfo->_piece_length / 20;	
	int payload = 0, len = piece_count + 1;
    unsigned char id = 5; 	

    printf("\n[%s:%s] - Length: %d Bitfield: %s", peer->ip, peer->port, ntohl(len), peer->swarm->bitfield);
    //len = htons(piece_count + 1);

    char* request = malloc(1 + 4 + len);
    memcpy(request, &len, 4);					payload += 4;
    memcpy(request + payload, &id, 1);			payload += 1;
    memcpy(request + payload, peer->swarm->bitfield, piece_count); 	payload += piece_count/8+1;

    send(peer->sockfd, request, payload, 0);		
    free(request);	
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
	piece_index = htonl(piece_index);

	memcpy(request, &len, 4); 						payload += 4;
	memcpy(request + payload, &id, 1);				payload += 1;
	memcpy(request + payload, &piece_index, 4); 	payload += 4;
	send(peer->sockfd, request, payload, 0);	
	free(request);
}

//hex formatting.
void printf_hexit(char* buf, int num)
{
	int i;

	for (i = 0; i < num; i++)
	{
		if (i > 0)
		{
			if ((i%8 == 0))
				printf("\t");

			if ((i%32 == 0))
				printf("\n");
		}
		printf("%02x  ", (unsigned char) buf[i]);
	}
	printf("\n------------------------------------------------------------------------");
}

//<len=0009+X><id=7><index><begin><block> 
static void inline seed_piece(char* buffer, int* num, int* msglen, peer_t* peer) 
{
	unsigned int index = 0, offset = 0, length = 0, header = 13, headerlen;
	unsigned char id = PIECE;
	memcpy(&index, buffer + 5, 4);
	memcpy(&offset, buffer + 9, 4);
	memcpy(&length, buffer + 13, 4);
	char* piece = malloc(ntohl(length));	//allocate length for piece
	char* packet = malloc(ntohl(length) + header);

	if ((*msglen > 16384 + header + 4) || ntohl(length) > 16384)
	{
		free(piece);
		free(packet);
		return;
	}

	//load piece data here, verify length to avoid bleeds.
	headerlen = header + ntohl(length) - 4;
	printf("[%s:%s] - Seeding Piece  [index: %d, offset: %d, length: %d]", peer->ip, peer->port, ntohl(index), ntohl(offset), ntohl(length));
	//memcpy(packet + 13, filebuffer + ntohl(offset) + (index * 16384), ntohl(length));
	headerlen = htonl(headerlen);							//length might be smaller on last piece!

	memcpy(packet, &headerlen, 4); 
	memcpy(packet + 4, &id, 1);
	memcpy(packet + 5, &index, 4);
	memcpy(packet + 9, &offset, 4);

	send(peer->sockfd, packet, ntohl(length) + header, 0);
	free(piece);
	free(packet);
}

//todo: fill one piece from the buffer, return num with the offset.
static void inline receive_piece(char* buffer, char* piebuffer, int* num, int* msglen, peer_t* peer)
{
	int index, offset, header = 13, tmp;

	memcpy(&index, buffer + 5, 4);
	memcpy(&offset, buffer + 9, 4);

	if (htonl(index) > 500 || htonl(index) < 0)
	{
		*num = 0;
		return;
	}

	while (*num < BLOCK_SIZE + 4)
	{
		tmp = recv(peer->sockfd, buffer + *num, DOWNLOAD_BUFFER - (*msglen + 4), 0);
		if (tmp < 1)
		{
			*num = 0;
			return;
		}
		*num += tmp;
	}

	memcpy(piebuffer + htonl(offset), buffer + header, BLOCK_SIZE);
	netstat_update(INPUT, BLOCK_SIZE, peer->info_hash);
	
	/* todo: uncomment when write_piece is working */
	//if (write_piece(peer->tinfo, (void*) piebuffer) == 0)
	//{
		printf("\n[%s:%s] - (Complete) Downloaded Piece! \tIndex: %d\tOffset: %d\tBF check: [%02x, %02x]", peer->ip, peer->port, htonl(index), 
									offset, (unsigned char) piebuffer[0], (unsigned char) piebuffer[16383]);
		fflush(stdout);
	//}
	//else
	//	printf("\n[%s:%s] - (Error) Piece Failed Checksum! \tIndex: %d\tOffset: %d\tBF check: [%02x, %02x]", peer->ip, peer->port, htonl(index), offset, 
	//								(unsigned char) piebuffer[0], (unsigned char) piebuffer[16383]);
}

//BT - Listener.
void* listener_tcp(void* arg)
{
	peer_t* peer = (peer_t*) arg;
	char recvbuf[DOWNLOAD_BUFFER];
	char* piebuffer = malloc(peer->tinfo->_piece_length);	//get piece size here.
	int num = 0, msglen;

	printf("\n[%s:%s]\tTCP Listener init.", peer->ip, peer->port);
	while (peer->sockfd != 0)
	{
		if (num > 4 || (num = recv(peer->sockfd, recvbuf, DOWNLOAD_BUFFER, 0)) > 0) //if buffer less than header size, issue a read op.
		{
			printf("\n[%s:%s] - Incoming Data: TYPE = %d LEN = %d MSG = %d\n", peer->ip, peer->port, (unsigned char) recvbuf[4], msglen, num);
			memcpy(&msglen, recvbuf, 4);
			msglen = htonl(msglen);
			
			if ((unsigned char) recvbuf[4] > 20)
				printf_hexit(recvbuf, num);

			if (msglen > 0) //K-A has 9 bytes set to zero, if not msglen is checked K-A will trigger choke. [ID read as 0]
			switch ((unsigned char) recvbuf[4])
			{
				case REQUEST: 		if (num < 17) { num = 0; continue; } seed_piece(recvbuf, &num, &msglen, peer);					break;		//continue: these methods will manage the buffer shifting.
				case PIECE: 		if (num < 17) { num = 0; continue; } receive_piece(recvbuf, piebuffer, &num, &msglen, peer); 	break;
				case HAVE: 			printf("\nHave!");											break; 
				case UNCHOKE: 		peer->choked = false;    printf("\nUnchoked!");				break;		//break: short messages, the buffer shifting is done below.
				case CHOKE:			peer->choked = true;     printf("\nChoked!");				break;
				case INTERESTED: 	peer->interested = true; printf("\nInterested!");			break;
				case NOT_INTERESTED:peer->interested = false;printf("\nNot Interested!");	 	break;
				case PORT:			printf("\nPort message received.");							break;
				case 84: 			printf("\nHandshake!");			msglen = num - 4;/*num = 0;*/				break; 	//Bittorrent Protocol...
				default: 			printf("\nUndefined!"); 		msglen = num - 4;/*num = 0;*/				break;
			}

			if (num - (msglen + 4) > 0 && msglen > 0 && msglen + 4 < DOWNLOAD_BUFFER)
			{
				num = num - (msglen + 4);
				printf("\nMem is on the move! msglen = %d, num = %d", msglen, num); fflush(stdout);
				memmove(recvbuf, recvbuf + msglen + 4, DOWNLOAD_BUFFER - (msglen + 4));
			}
			else
				num = 0;
		} 
		else if (num < 1)
			break;
	}
	printf("\n[%s:%s] - Disconnected while reading.", peer->ip, peer->port); fflush(stdout);
	close(peer->sockfd);
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
		printf("\n[%s:%s]\tStarting peer listener..", peer->ip, peer->port);


	printf("\n[%s:%s] - Connected!\n", peer->ip, peer->port); fflush(stdout);
	//printf("\n[sockfd = %d], piece length = %lld", peer->sockfd, peer->tinfo->_piece_length);
	sleep(1);
	handshake(peer, peer->info_hash, peer->peer_id);
	//sleep(1);
	//bitfield(peer);  //todo: uncomment when scan_all is working
	sleep(2);
	message(peer, INTERESTED);
	sleep(2);
	message(peer, UNCHOKE);
	printf("\n[%s:%s] - Handshake sequence finished.", peer->ip, peer->port); fflush(stdout);


	int block, index, piecelen = peer->tinfo->_piece_length, blockcount;
	while (peer->sockfd != 0) //&& index > -1, stop this thread when the peer is no longer interesting, close the socket but do not change sockfd value.
	{

		//get not downloaded piece, request for every block in piece.
		index = random()%8; //get piece index here.
		blockcount = piecelen / BLOCK_SIZE; //how many full blocks.
		block = 0;
		while (index > -1 && block < blockcount && peer->choked == false)
		{
			//printf("\n[%s:%s] - Requesting: block = %d, count = %d, pLen = %d, bLen = %d", peer->ip, peer->port, block, blockcount, piecelen, BLOCK_SIZE);
			request(peer, htonl(index), htonl(block * BLOCK_SIZE), htonl(BLOCK_SIZE));	//request 16384 if not last piece
			block++;
			//sleep(1); //issue /have on download complete. not implemented.
		}

		usleep(30000);
		//break;
	}
	printf("\n[%s:%s] - Peer disconnected while writing.", peer->ip, peer->port);
	//shutdown the listener, free the peer, close the sockfd.

	return arg;
}
                                                                         