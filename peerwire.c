/* peerwire.c
 * 2014-05-06
 * Robin Duda
 *  Peerwiring.
 */

#include "peerwire.h"
 /*
	todo:
		peers keep track of the last informed bitfield (XOR), broadcast changes with have message, then update the local bitfield.
		keep track of the remote peers pieces with a bitfield, update on have.
		NOT local bitfield and AND with remote peer to find pieces to download,
		when downloading a piece set flag in swarm to completed during download.
		in swarm: protect bitfield by lock.
 */

//print some data, hex style! (used for checking the buffers during dev.)
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

//handshake with the peer, informing them of any extensions in use.
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

//request download for a piece.
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

//received a bitfield message. [todo: implementation. function stubs are gross. top priority item WIP]
void peer_bitfield(char* recvbuf, int* num, int *msglen, peer_t* peer)
{
	int piece_count = peer->tinfo->_hash_length / 20;
	int i, header = 5;

	for (i = 0; i < piece_count / 8 + 1; i++)
		peer->bitfield_peer[i] = bitfield_reverse(recvbuf[header + i]);
}

//tell peers which pieces are in your posession.
void bitfield(peer_t* peer)
{
	int piece_count = peer->tinfo->_hash_length / 20;	
	int payload = 0, len = htonl(piece_count / 8 + 2);
    unsigned char id = 5; 
	char* revfield = malloc(ntohl(len) - 1);
	int i;

	for (i = 0; i < piece_count / 8 + 1; i++)
	  revfield[i] = bitfield_reverse(peer->swarm->bitfield[i]);    	

    char* request = malloc(4 + ntohl(len));
    memcpy(request, &len, 4);					payload += 4;
    memcpy(request + payload, &id, 1);			payload += 1;
    memcpy(request + payload, revfield, piece_count / 8 + 1); 	payload += piece_count / 8 + 1;

    send(peer->sockfd, request, payload, 0);		
    free(request);
    free(revfield);	
}

//messages: choke, unchoke, interested, not interested]
void message(peer_t* peer, unsigned char message)
{
	int payload = 0, len = htonl(1);
    char* request = malloc(4 + 1);

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


void peer_have(char* recvbuf, peer_t* peer)
{
	int header = 5, index;

	memcpy(&index, recvbuf + header, 4);
	index = ntohl(index);
	bitfield_set(peer->bitfield_peer, index);
}

//call when a piece has been successfully downloaded (when bitfield_sync differs from bitfield_swarm)
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

//upload a piece to the peer!
static void inline seed_piece(char* buffer, int* num, int* msglen, peer_t* peer) 
{
	unsigned int index = 0, offset = 0, length = 0, header = 13, headerlen;
	unsigned char id = PIECE;
	memcpy(&index, buffer + 5, 4);
	memcpy(&offset, buffer + 9, 4);
	memcpy(&length, buffer + 13, 4);
	char* piece = malloc(ntohl(length));	
	char* packet = malloc(ntohl(length) + header);

	//critical to verify input data. if the index/length are out of index range, a client could read our memory.
	if ((*msglen > BLOCK_SIZE + header + 4) || ntohl(length) > BLOCK_SIZE)
	{
		free(piece);
		free(packet);
		return;
	}

	headerlen = header + ntohl(length) - 4;
	//printf("[%s:%s] - Seeding Piece  [index: %d, offset: %d, length: %d]", peer->ip, peer->port, ntohl(index), ntohl(offset), ntohl(length)); [WIP]
	headerlen = htonl(headerlen);							//length might be smaller on last piece!

	memcpy(packet, &headerlen, 4); 
	memcpy(packet + 4, &id, 1);
	memcpy(packet + 5, &index, 4);
	memcpy(packet + 9, &offset, 4);

	send(peer->sockfd, packet, ntohl(length) + header, 0);
	free(piece);
	free(packet);
}

//receive an incoming piece, direct it to the file manager.
static void inline receive_piece(char* buffer, char* piebuffer, int* num, int* msglen, peer_t* peer)
{
	int index, offset, header = 13, tmp, size;

	memcpy(&index, buffer + 5, 4);
	memcpy(&offset, buffer + 9, 4);
	peer->queued_pieces--;

	//critical to verify input data. if the index/length are out of index range, a client could write in our memory.
	if (htonl(index) >= (peer->tinfo->_hash_length / 20) || htonl(index) < 0 || *msglen < 0 || 
		*msglen > peer->tinfo->_piece_length + 9 || htonl(offset) + *msglen -9 > (peer->tinfo->_piece_length))
	{
		*num = 0;
		return;
	}

	while (*num < *msglen + 4)
	{
		tmp = recv(peer->sockfd, buffer + *num, DOWNLOAD_BUFFER - (*msglen + 4), 0);
		if (tmp < 1)
		{
			*num = 0;
			return;
		}
		*num += tmp;
	}

    char* output = malloc(peer->tinfo->_piece_length); 
	memcpy(piebuffer + htonl(offset), buffer + header, *msglen - 9);
	memcpy(output, piebuffer, peer->tinfo->_piece_length);
	netstat_update(INPUT, *msglen - 9, peer->info_hash);

	//printf("\nWriting piece!"); fflush(stdout);
	//if piece already exists, do not download.																		
	if (bitfield_get(peer->swarm->bitfield, htonl(index)) == 0)
	{
			if (htonl(index) == (peer->swarm->tinfo->_hash_length / 20) - 1)
				size =  (peer->tinfo->_total_length % peer->tinfo->_piece_length);
			else 
				size = peer->tinfo->_piece_length;

		if (write_piece(peer->tinfo, (void*) output, htonl(index), size) == 0)
		{
			bitfield_set(peer->swarm->bitfield, htonl(index));
			bitfield_set(peer->bitfield_peer, htonl(index));
		}
	}
	else
		free(output);

}

//BT - Listener.
//from the receive buffer read a packet/message, then shift the buffer for the size of the read packet.
//when the buffer is empty, or there isn't enough data for a header, more data will be read into the buffer.
void* listener_tcp(void* arg)
{
	peer_t* peer = (peer_t*) arg;
	char recvbuf[DOWNLOAD_BUFFER];
	char* piebuffer = malloc(peer->tinfo->_piece_length);	
	int num = 0, msglen;
	bool packet_queued = false;

	while (peer->sockfd != 0)
	{
		if (packet_queued == true || (num += recv(peer->sockfd, recvbuf + num, DOWNLOAD_BUFFER, 0)) > 0) 
		{
			memcpy(&msglen, recvbuf, 4);
			msglen = ntohl(msglen);

			if (num < msglen + 4 && (unsigned char) recvbuf[4] != HANDSHAKE) //handshake messages have custom length.
				continue;

			if (msglen > 0) //if message length is 0 the message is a Keep-Alive, ignore it.
			switch ((unsigned char) recvbuf[4])
			{						
				case REQUEST: 		if (num < 17) { num = 0; continue; } seed_piece(recvbuf, &num, &msglen, peer);					break;		
				case PIECE: 		if (num < 17) { num = 0; continue; } receive_piece(recvbuf, piebuffer, &num, &msglen, peer); 	break;
				case HAVE: 			peer_have(recvbuf, peer);									break; 
				case UNCHOKE: 		peer->choked = false; 										break;		
				case CHOKE:			peer->choked = true;     									break;
				case INTERESTED: 	peer->interested = true; 									break;
				case NOT_INTERESTED:peer->interested = false;								 	break;
				case BITFIELD:      peer_bitfield(recvbuf, &num, &msglen, peer);				break;
				case PORT:			/**/														break;
				case HANDSHAKE: 	msglen = 1 + (unsigned char) recvbuf[0] + 8 + 20 + 20 - 4;  break; 	
				case EXTENDED:      /**/														break;
				default: 			/**/											   			break;
			}		

			if (num - (msglen + 4) > 0 && msglen > 0 && msglen + 4 < DOWNLOAD_BUFFER)
			{
				num = num - (msglen + 4);
				memmove(recvbuf, recvbuf + msglen + 4, DOWNLOAD_BUFFER - (msglen + 4));			//shift the buffer.
			}
			else
				num = 0;	//the buffer was corrupted, reset. on wireless networks the buffer will get corrupted when trying to send data too fast.
							//by setting num to 0, we may recover from a buffer corruption/out of sync.

			memcpy(&msglen, recvbuf, 4);
			msglen = ntohl(msglen);
			if (msglen + 4 <= num)
			{
				packet_queued = true;
			}
			 else
			 	packet_queued = false;
		} 
		else if (num < 1)	//socket returned with error message or the peer disconnected, stop listening.
			break;
	}
	close(peer->sockfd);
	free(peer->bitfield_sync);
	free(peer->bitfield_peer);
	peer->sockfd = 0;
	return arg;
}

//get a piece not downloaded non linearly to utilize multiple peers. 
//[todo: add queue system and get rid of this nasty hack!!!]
bool missing_piece(peer_t* peer, int* index)
{
	int piece_count = peer->tinfo->_hash_length / 20;
	int scan_count = piece_count + 1;
	bool missing = false;

	*index = rand()%piece_count - 1;

	while (scan_count > 0)
	{
		*index += 1;
		scan_count--;

		if (*index == piece_count)
			*index = 0;

		if (bitfield_get(peer->swarm->bitfield, *index) == 0)
		{
			missing = true;

			if (bitfield_get(peer->bitfield_peer, *index) == 1)
			{
				//bitfield_clear(peer->bitfield_peer, *index); //[todo: prevent asking for the same piece multiple times from the same peer. - 
				return missing;									// - uncommenting this line will cause a stall if theres only one peer.]
			}
		}
	}
	*index = -1;
	return missing;
}

//this thread may be invoked from the swarm listener, in which case the sock is already set up.
void* peerwire_thread_tcp(void* arg)
{
    struct addrinfo hints, *res;
    int piece_count = swarm->tinfo->_hash_length / 20;
    pthread_t listen_thread;
    peer_t* peer = (peer_t*) arg;

    //set up connection specific flags
    peer->choking = 1;
    peer->choked = 1;
    peer->interested = 0;
    peer->interesting = 0;
    peer->queued_pieces = 0;

    //sync field is peer specific, diff with swarm->bitfield to issue have messages. peer is remote.
    peer->bitfield_peer = malloc((piece_count / 8) + 1);
    peer->bitfield_sync = malloc((piece_count / 8) + 1);
    memcpy(peer->bitfield_sync, peer->swarm->bitfield, (((peer->tinfo->_hash_length / 20) / 8) + 1));
    memset(peer->bitfield_peer, 0, ((swarm->tinfo->_hash_length / 20) / 8) + 1);

    //not already connected, we are the initiator of this connection.
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
			return arg; 
	}

	//the peer listener thread will read and handle incoming messages.
    if (pthread_create(&listen_thread, NULL, listener_tcp, peer))
		printf("\n[%s:%s]\tStarting peer listener.. Failed!", peer->ip, peer->port);

	handshake(peer, peer->info_hash, peer->peer_id); 	
	bitfield(peer);  									
	message(peer, INTERESTED);							sleep(1); //[todo: add logic for interested message]
	message(peer, UNCHOKE);								sleep(1); //[todo: add logic for unchoke message]

	int block, index = 0, piecelen = peer->tinfo->_piece_length, blockcount, size = BLOCK_SIZE;
	while (peer->sockfd != 0)
	{
		blockcount = piecelen / BLOCK_SIZE; 
		block = 0;

		if (missing_piece(peer, &index) == false)
			break;

		if (index == -1 || peer->queued_pieces >= PIECE_QUEUE)
		{
			usleep(PIECE_WAIT * 1000);
			continue;
		}

		while (block < blockcount && peer->choked == false && bitfield_get(peer->swarm->bitfield, index) == 0) 
		{ 
			if (index == (peer->swarm->tinfo->_hash_length / 20) - 1 && ((peer->tinfo->_total_length % peer->tinfo->_piece_length) / BLOCK_SIZE) == block)
			{
				size =  ((peer->tinfo->_total_length % peer->tinfo->_piece_length) % BLOCK_SIZE);
				blockcount = -1;
			}
			else 
				size = BLOCK_SIZE;

			peer->queued_pieces++;
			request(peer, htonl(index), htonl(block * BLOCK_SIZE), htonl(size));
			block++;
		}

		if (peer->choked == true)
			sleep(CHOKE_BACKOFF);
	}
	return arg;
}
                                                                         