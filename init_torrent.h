#ifndef INIT_TORRENT_H
#define INIT_TORRENT_H
#include "bencodning.h"
#include "createfile.h"
#include <unistd.h>
#include <pthread.h>

int init_torrent (char *, torrent_info *);

#endif