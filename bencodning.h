#ifndef BENCODNING_H
#define BENCODNING_H

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
typedef struct metainfodecode
{
	char _announce[250];
	char _announce_list [15][250];
	long long int _creation_date;
	char _comment [250];
	char _created_by[250];

	long long int _piece_length;
	int _private;

	char _file_name [250];
	long long int _file_length;
	char _file_MD5 [32];

	long int _hash_length;
	char _pieces [5000][20];
} torrent_info;
/*
int init_file (char *, FILE *);
void list_handler(FILE *, char *, torrent_info *);
char read_one_char (FILE *);
void read_specific_length (FILE *, int, char *);
int read_length_of_next (FILE *);
int dictonaty_encoding (FILE *);
void hash_handler(FILE *, torrent_info *);
void complete_dictonarry (char *, char *, torrent_info *);
void dictonarry_handler (FILE *, torrent_info *, char *);
void announce_list_handler(FILE *, torrent_info *);
void int_handler(FILE *, char *, torrent_info *);
*/
extern int decode_bencode(int,char **)

#endif