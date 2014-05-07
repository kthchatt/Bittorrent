/*
This is the geader file for "bencodning.c". 
The function decode_bencode takes a char string as file name
and a torrent_info struct as data storage with information about
the torrent. 

Written by Gustaf Nilstadius on behalf of KTH STH. 
Used by groupe 42 in project. 

Feel fre to use this code as you want. 
*/
#ifndef BENCODNING_H
#define BENCODNING_H

#include <openssl/sha.h>
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
	int _multi_file;

	char _info_hash[21];
	long int _location;


	long long int _piece_length;
	int _private;
//Common for singel and multi, singel uses only first long long int. 
	long long int _file_length [4000];

//For singel file torrent
	char _file_name [250];
	char _file_MD5 [32];

//For multi file torrent
	int _number_of_files;
	char _file_path[4000][500];

	long int _hash_length;
	char _pieces [5000][20];
} torrent_info;

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
void path_handler(char *, int, torrent_info *);
void info_hash (FILE *, torrent_info *);

int decode_bencode(char *, torrent_info *);

#endif