//gcc readpiece.c -c -o readpiece.o -Wall
#include "readpiece.h"

void *readpiece(torrent_info *torrent, int piece_index){
	int start_in_file = torrent->_piece_length * piece_index, total_bytes_read = 0;
	int i, first_file_to_open, bytes_read, bytes_to_read = torrent->_piece_length, found_piece = -1;
	int number_of_pieces = (int) torrent->_hash_length /20;
	//long long int bytes_to_write = torrent->_piece_length;

	//if last piece, get size of last piece. ~RD
	if (number_of_pieces == piece_index + 1)
		bytes_to_read = torrent->_total_length % torrent->_piece_length;

	if (number_of_pieces < piece_index){
		return NULL;
	}

	void *piece = malloc(torrent->_piece_length);
	void *copy_piece = piece;


	i = 0;
	while((start_in_file - torrent->_file_length[i]) > 0){
		start_in_file -= torrent->_file_length[i++];
	}

	first_file_to_open = i;
	//fprintf(stderr, "Piece length is %lld \n", bytes_to_read);
	while(bytes_to_read > 0 && first_file_to_open < torrent->_number_of_files){
		FILE *fp;
		fp = fopen(torrent->_file_path[first_file_to_open], "rb+");
		if (fp == NULL){
			fprintf(stderr, "Error opening file\n");
			return NULL;
		} else {
			//fprintf(stderr, "File %s is open\n", torrent->_file_path[first_file_to_open]);
		}
		fseek(fp, start_in_file, SEEK_SET);
		start_in_file = 0;

		bytes_read = fread(piece, 1, bytes_to_read, fp);
		//fprintf(stderr, "Bytes Written: %d\n", bytes_read);
		bytes_to_read -= bytes_read;
		piece += bytes_read;
		total_bytes_read += bytes_read;
		first_file_to_open++;
		fclose(fp);
	}

	//bytes to read was not initialized, fixed: now working. ~RD

	unsigned char hash[20];
	SHA1(copy_piece, total_bytes_read, (unsigned char *)hash);
	
	for (i = 0; i < number_of_pieces; i++){
		if (hashncmp(hash, torrent->_pieces[i], 20) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "The hash is not found\n");
		return NULL;
	} else {
		return copy_piece;
	}
		
}