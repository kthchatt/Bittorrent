#ifndef _torrent_h
#define _torrent_h

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include "tracker.h"
#include "bencodning.h"

// get info hash from torrent file
char *getInfoHash(char *torrentPath);
// copy torrent to directory, announce to tracker and init transfer
int addTorrent(char *torrentPath);

#endif 