#include "SearchPice.h"
//gcc SearchPice.c -c -o SearchPice.o -lssl -lcrypto
/*void main (){

	search_single_file("Arbete och teknik på människans villkor_student.pdf", "00000000000000000000", 56000);

}*/

int scan_all (torrent_info *torrent, unsigned char *bitstring) {
	int number_of_pieces = (torrent->_hash_length / 20);
	int found = -1, j = 0, bytes_read = 0, total_bytes_read = 0;
	int i, first_file_to_open, toalloc = (number_of_pieces/8)+1;
	int piece_length = torrent->_piece_length;

	memset(bitstring, 0, toalloc);

	unsigned char hash[20]; printf("\ndbg1");fflush(stdout);

	void *piece;
	piece = malloc(piece_length);
	void *copy_piece = piece;
	memset(piece, 0, piece_length);

	for(j = 0; j < number_of_pieces; j++){printf("\ndbg2");fflush(stdout);
		int bytes_to_read = piece_length;
		int start_in_file = torrent->_piece_length * j;
		i = 0;
		while((start_in_file - torrent->_file_length[i]) > 0){
			start_in_file -= torrent->_file_length[i++];
		}

		first_file_to_open = i;printf("\ndbg3");fflush(stdout);

		while(bytes_to_read > 0 && first_file_to_open < torrent->_number_of_files){
			printf("\ndbg3.1"); fflush(stdout);
			FILE *fp;
			fp = fopen(torrent->_file_path[first_file_to_open], "rb+");
			//fprintf(stderr, "File: %s\n", torrent->_file_path[first_file_to_open]);printf("\ndbg1");fflush(stdout);
			if (fp == NULL){
				fprintf(stderr, "Error opening file\n");
				return -1;
			} else {
				//fprintf(stderr, "File %s is open\n", torrent->_file_path[first_file_to_open]);
			}
			fseek(fp, start_in_file, SEEK_SET);
			start_in_file = 0;

			bytes_read = fread(piece, 1, bytes_to_read, fp);printf("\ndbg4");fflush(stdout);
			//fprintf(stderr, "Bytes Written: %d\n", bytes_read);
			bytes_to_read -= bytes_read;
			piece += bytes_read;
			total_bytes_read += bytes_read;
			first_file_to_open++;
			fclose(fp);
		}

		printf("\ndbg4.1: total_bytes_read = %d", total_bytes_read); fflush(stdout);
		piece = copy_piece;
		SHA1(piece, total_bytes_read, hash);printf("\ndbg5");fflush(stdout);
		if (j%8 == 0 && j != 0){
			bit_field++;
		}
		printf("\ndbg4.2"); fflush(stdout);
		if (hashncmp(hash, torrent->_pieces[j], 20) == 0){
			printf("\ndbg6");fflush(stdout);
			*bit_field |= (1<<(j%8));
			if (found == -1){
				found  = j;
			}
		}
		total_bytes_read = 0;
		
	}
	printf("\ndbg7");fflush(stdout);
	free(copy_piece);
	return found;
}

int search_multi_file (torrent_info *torrent, unsigned char *original_hash){
	FILE *sfp[250];
	int found =0, i = 0, total_to_load = 0, loaded_files = 0, start_from = 0, stop_at = 0, bytes_read = 0, total_loaded_files = 0;
	char *file_name;
	unsigned char hash[20];
    long long int piece_length = torrent->_piece_length;

    unsigned char *data;
	data = (unsigned char *) malloc(piece_length);
	memset(data, 0, piece_length);
    do{
    	i = stop_at + 1;
    	while (total_to_load < piece_length && i < torrent->_number_of_files){
    		total_to_load = total_to_load + torrent->_file_length[i];
    		i++;
    	}
    	stop_at = i;
    	for (i = start_from; i <= stop_at; i++)
    	{
    		file_name = torrent->_file_path[i];
    		file_name++;
    		fprintf(stderr, "File name: %s\n", file_name);
    		sfp[i - start_from] = fopen(file_name, "r");
    		if (sfp[i] != NULL){
			} else {
				fprintf(stderr, "Error opening file\n");
				return(-1);
			}
    	}
    	loaded_files = stop_at - start_from;
    	fprintf(stderr, "Files are open\n");
    	for (i = 0; i < loaded_files; i++)
    	{
    		bytes_read = fread(data, 1, total_to_load, sfp[i]);
    		fprintf(stderr, "Bytes read %d, Number of bytes should be read %d\n", bytes_read, total_to_load);
    		total_to_load -= bytes_read;
    	}
    	total_loaded_files += loaded_files;

		SHA1((const unsigned char *)data, bytes_read, hash);

		if (hashncmp(hash, original_hash, 20) == 0){
			found = 1;
			return 1;
			break;
		}

    }while (found != 1 && total_loaded_files < torrent->_number_of_files);
    return 0;
}



int search_single_file (char *file_name, unsigned char *original_hash, long long int piece_length){
	int found = 0, pice_index = -1, i = 0, bytes_read;
	char *data;
	data = (char *) malloc(piece_length);
	memset(data, 0, piece_length);
	unsigned char hash[21];
	memset(hash, 0, 21);

	fprintf(stderr, "file name is %s\n", file_name);
	FILE *sfp;
	sfp = fopen(file_name, "r");
	if (sfp != NULL){
	} else {
		fprintf(stderr, "Error opening file\n");
		return(-1);
	}
	fprintf(stderr, "File is open\n");
	fprintf(stderr, "Original hash = %s\n", original_hash);

	do{
		pice_index++;
		bytes_read = fread(data, 1, piece_length, sfp);
		fprintf(stderr, "Bytes read %d, Number of bytes should be read %lld\n", bytes_read, piece_length);
		SHA1((const unsigned char *)data, bytes_read, hash);


		fprintf(stderr, "Generated hash = \t");
		for (i = 0; i < 20; i++){
			fprintf(stderr, "%02x", (unsigned char) hash[i]);
		}
		fprintf(stderr, " \nOriginal hash = \t");
		for (i = 0; i < 20; i++){
			fprintf(stderr, "%02x", (unsigned char) original_hash[i]);
			//original_hash++;
		}
		//original_hash-=20;
		fprintf(stderr, "\n\n");


		if (hashncmp(hash, original_hash, 20) == 0){
			found = 1;
			break;
		}
	} while (found != 1 && pice_index < 10000 && feof(sfp) == 0);
	free(data);
	if(found == 1){
		return 1;
	} else {
		return 0;
	}

}
