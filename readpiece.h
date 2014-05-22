#ifndef READPIECE_H
#define READPIECE_H

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "bencodning.h"
//Reads one piece and returns a void pointer to the memory address of the piece. 
void *readpiece(torrent_info *, int);

#endif