/*gcc writepiece.c -c -o writepiece.o -Wall
*/
#include "writepiece.h"

													//index is already known when calling write_piece, piece_length is less for the last piece. ~RD
int write_piece (torrent_info *torrent, void *piece, int index, int piece_length){
	printf("\ntest from write_piece index: %d length: %d", index, piece_length); fflush(stdout);
	pthread_t write_piece_pthread_t;
	write_piece_struct* torrent_piece = malloc(sizeof(write_piece_struct));
	torrent_piece->torrentptr = torrent;
	torrent_piece->pieceptr = piece;
	torrent_piece->length = piece_length;
	/*int i, found_piece = -1, number_of_pieces; */; //unused ~RD
	unsigned char hash[20];

	//------ added ~RD
	SHA1(piece, torrent_piece->length, hash);				
	if (hashncmp(hash, torrent->_pieces[index], 20) == 0) {
		torrent_piece->index = index;
		pthread_create(&write_piece_pthread_t, NULL, write_piece_thread, torrent_piece);
		return 0;	//hash found, success.
	}
	else {
		fprintf(stderr, "The hash is not found (PRINT FROM PROCESS)");
		free(piece);
		return -1;	//hash not found, failure.
	}
	//--------

	//removed, looping through all the pieces not required, index already known. ~RD
	//number_of_pieces = (int) torrent->_hash_length /20;
	//SHA1(piece, torrent_piece->length, hash);	
	/*for (i = 0; i < number_of_pieces; i++){
		if (hashncmp(hash, torrent->_pieces[i], 20) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "The hash is not found (PRINT FROM PROCESS)\n");
		free(piece);
		return 1;
	} else {
		pthread_create(&write_piece_pthread_t, NULL, write_piece_thread, torrent_piece);
		return 0;
	}*/
}

//todo: checking for the hash a second time is redundant, remove from thread.  ~RD
void *write_piece_thread(void *torrent_piece){
	fprintf(stderr, "Test from thread\n");
	write_piece_struct *local_struct = (write_piece_struct *) torrent_piece;
	torrent_info *torrent = local_struct->torrentptr;
	char* piece_copy = local_struct->pieceptr;
	/*void *piece = local_struct->pieceptr;*/

	fprintf(stderr, "Thread: This is a second test\n");

	//void *piece_copy;
	//piece_copy = piece;
	fprintf(stderr, "Thread: This is a third test\n");
	int i, /*number_of_pieces, found_piece = -1,*/ first_file_to_open = 0,  bytes_written = 0, start_in_file = 0;
	fprintf(stderr, "Thread: This is a fourth test\n");
	fprintf(stderr, "Thread: Piece length is %lld\n", torrent->_piece_length);
	long long int bytes_to_write = local_struct->length;	//length of the piece, accounts for the last piece which is shorter. ~RD

	fprintf(stderr, "Thread: This is a fifth test\n");


	//removed redundant code. (same thing has already been done in write_piece) ~RD
	/*unsigned char hash[20];
	SHA1(piece, torrent->_piece_length, (unsigned char *) hash); //todo: the last piece does not have _piece_length! ~RD
	number_of_pieces = (int) torrent->_hash_length /20;

	for (i = 0; i < number_of_pieces; i++){
		if (hashncmp(hash, torrent->_pieces[i], 20) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "Thread: The hash is not found\n");
		return torrent_piece;
	}*/

	/*Remove comment below for testning.
	*/
	//found_piece = 1;

	start_in_file = torrent->_piece_length * local_struct->index;

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
			fprintf(stderr, "File %s is open\n", torrent->_file_path[first_file_to_open]);
		}
		fseek(fp, start_in_file, SEEK_SET);
		start_in_file = 0;

		bytes_written = fwrite(piece_copy, 1, bytes_to_write, fp);
		//fprintf(stderr, "Bytes Written: %d\n", bytes_written);
		bytes_to_write -= bytes_written;
		piece_copy += bytes_written;
		first_file_to_open++;
		fclose(fp);
	}
	free(local_struct->pieceptr);
	free(torrent_piece);  
	return torrent_piece;
}