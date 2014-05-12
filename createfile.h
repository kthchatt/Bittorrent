/*
This is header file for createfile.c. 
The files are created in the current working directory of the program. 
crete_file cretes all the files, and create_file_status returns a double on the current procentage of created files.
*/
#ifndef CREATEFILE_H
#define CREATEFILE_H

#define TO_CREATE 1000000

#include "bencodning.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

double create_file_status(torrent_info *);
int create_file(torrent_info *);


#endif