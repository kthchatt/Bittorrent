#ifndef WRITEPIECE_H
#define WRITEPIECE_H

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "bencodning.h"

typedef struct writepiecestruct
{
	torrent_info *torrentptr;
	void *pieceptr;
	int length, index;
} write_piece_struct;

void *write_piece_thread(void *);

int write_piece(torrent_info *torrent, void *, int index, int piece_length);

#endif