/*
*/
#include "writepiece.h"


int write_piece (torrent_info *torrent, void *piece){
	pthread_t write_piece_pthread_t;
	write_piece_struct torrent_piece;
	torrent_piece.torrentptr = torrent;
	torrent_piece.pieceptr = piece;

	int i, number_of_pieces, found_piece = -1;
	char hash[20];
	SHA1(piece, torrent->_piece_length, hash);
	number_of_pieces = (int) torrent->_hash_length /20;

	for (i = 0; i < number_of_pieces; i++){
		if (strncmp(hash, torrent->_pieces[i], 20) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "The hash is not found (PRINT FROM PROCESS)\n");
		free(piece);
		return 1;
	} else {
		pthread_create(&write_piece_pthread_t, NULL, write_piece_thread,&torrent_piece);
		return 0;
	}
}

void *write_piece_thread(void *torrent_piece){
	fprintf(stderr, "Test from thread\n");
	write_piece_struct *local_struct;
	local_struct = torrent_piece;
	torrent_info *torrent = local_struct->torrentptr;
	void *piece = local_struct->pieceptr;

	fprintf(stderr, "Thread: This is a second test\n");

	void *piece_copy;
	piece_copy = piece;
	fprintf(stderr, "Thread: This is a third test\n");
	int i, number_of_pieces, found_piece = -1, first_file_to_open = 0,  bytes_written = 0, start_in_file = 0;
	fprintf(stderr, "Thread: This is a fourth test\n");
	fprintf(stderr, "Thread: Piece length is %lld\n", torrent->_piece_length);
	long long int bytes_to_write = torrent->_piece_length;

	fprintf(stderr, "Thread: This is a fifth test\n");

	char hash[20];
	SHA1(piece, torrent->_piece_length, hash);
	number_of_pieces = (int) torrent->_hash_length /20;

	for (i = 0; i < number_of_pieces; i++){
		if (strncmp(hash, torrent->_pieces[i], 20) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "Thread: The hash is not found\n");
		return torrent_piece;
	}
	/*Remove comment below for testning.
	*/
	//found_piece = 1;

	start_in_file = torrent->_piece_length * found_piece;

	i = 0;
	while((start_in_file - torrent->_file_length[i]) > 0){
		start_in_file -= torrent->_file_length[i++];
	}
	//fprintf(stderr, "Start in file is : %d\n", start_in_file);
	first_file_to_open = i;
	//fprintf(stderr, "Piece length is %lld \n", bytes_to_write);
	while(bytes_to_write > 0){
		FILE *fp;
		fp = fopen(torrent->_file_path[first_file_to_open], "rb+");
		//fprintf(stderr, "File: %s\n", torrent->_file_path[first_file_to_open]);
		if (fp == NULL){
			fprintf(stderr, "Error opening file\n");
			return torrent_piece;
		} else {
			//fprintf(stderr, "File %s is open\n", torrent->_file_path[first_file_to_open]);
		}
		fseek(fp, start_in_file, SEEK_SET);
		start_in_file = 0;

		bytes_written = fwrite(piece, 1, bytes_to_write, fp);
		//fprintf(stderr, "Bytes Written: %d\n", bytes_written);
		bytes_to_write -= bytes_written;
		piece += bytes_written;
		first_file_to_open++;
		close(fp);
	}
	free ( piece_copy );
	return torrent_piece;
}