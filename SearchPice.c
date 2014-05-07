#include "SearchPice.h"

/*void main (){

	search_single_file("Arbete och teknik på människans villkor_student.pdf", "00000000000000000000", 56000);

}*/

int scan_all (torrent_info *torrent, char *bitstring) {
	int total = (torrent->_hash_length / 20);
	int found =0, j = 0, total_to_load = 0, loaded_files = 0, start_from = 0, stop_at = 0, bytes_read = 0, total_loaded_files = 0;
	int fileindex = 0, nom_of_files_to_load = 0;
	int i, toalloc = (total/8)+1
	long int piece_length = torrent->_piece_length;

	bitstring = malloc(toalloc);
	memset(bitstring, 0, toalloc);

	char hash[21];

	char *data;
	data = (char *) malloc(piece_length);
	memset(data, 0, piece_length);

	FILE *sfp[250];
	for (i = 0; i < total; i++){
		start_from = stop_at + 1;

		/*Calculates what to load, exits loop if trying to load more than what exists.*/
		while (total_to_load < piece_length && fileindex < torrent->_number_of_files){
    		total_to_load = total_to_load + torrent->_file_length[fileindex];
    		nom_of_files_to_load++;
    		fileindex++;
    	}
    	stop_at = fileindex;
    	for (j = start_from; j <= fileindex; j++)
    	{
    		file_name = &torrent->_file_path[j];
    		file_name++;
    		fprintf(stderr, "File name: %s\n", file_name);
    		sfp[j - start_from] = fopen(file_name, "r");
    		if (sfp[j] != NULL){
			} else {
				fprintf(stderr, "Error opening file\n");
				return(-1);
			}
    	}

    	loaded_files = stop_at - start_from;
    	fprintf(stderr, "Files are open\n");

    	total_to_load = piece_length;
    	for (j = 0; j < loaded_files; j++)
    	{
    		bytes_read = fread(data, 1, total_to_load, sfp[j]);
    		fprintf(stderr, "Bytes read %d, Number of bytes should be read %d\n", bytes_read, total_to_load);
    		total_to_load -= bytes_read;
    	}
    	total_loaded_files += loaded_files;

		SHA1(data, bytes_read, hash);

		if (strncmp(hash, original_hash, 20) == 0){
//TODO, fix so that bitstring is used 1 = have piece, 0 = missing piece. 

			found = 1;
			return 1;
			break;
		}


	}
	free(data);
	return total;
}

int search_multi_file (torrent_info *torrent, char *original_hash){
	FILE *sfp[250];
	int found =0, i = 0, total_to_load = 0, loaded_files = 0, start_from = 0, stop_at = 0, bytes_read = 0, total_loaded_files = 0;
	char *tmpptr;
	char *file_name;
	char hash[21];
    long long int piece_length = torrent->_piece_length;

    char *data;
	data = (char *) malloc(piece_length);
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
    		file_name = &torrent->_file_path[i];
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

		SHA1(data, bytes_read, hash);

		if (strncmp(hash, original_hash, 20) == 0){
			found = 1;
			return 1;
			break;
		}

    }while (found != 1 && total_loaded_files < torrent->_number_of_files);
    return 0;
}



int search_single_file (char *file_name, char *original_hash, long long int piece_length){
	int found = 0, pice_index = -1, i = 0, bytes_read;
	char *data;
	data = (char *) malloc(piece_length);
	memset(data, 0, piece_length);
	char hash[21];
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
		SHA1(data, bytes_read, hash);


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


		if (strncmp(hash, original_hash, 20) == 0){
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
