#ifdef WRITEPIECE_H
#define WRITEPIECE_H

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int write_piece(torrent_info *torrent, void *);

#endif