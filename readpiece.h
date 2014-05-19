#ifndef READPIECE_H
#define READPIECE_H

#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bencodning.h"

void *readpiece(torrent_info *, int);

#endif