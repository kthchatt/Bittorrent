/*
This is header file for SearchPice.c. 
Use search_single_file if the torrent is single file torrent.
It's faster than search_multi_file. 

search_multi_file Searchers all of the torrent files after a predefined hash.

RETURN VALUE:
Returns 1 if hash is found.
Returns 0 if has is NOT found.
*/
#ifndef SEARCHP_H
#define SEARCHP_H

#include "bencodning.h"
#include <openssl/sha.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int scan_all (torrent_info *, char *);
int search_multi_file (torrent_info *, unsigned char *);
int search_single_file (char *, unsigned char *, long long int );


#endif