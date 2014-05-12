/*
*/

int write_piece(torrent_info *torrent, void *piece){
	int i, number_of_pieces, start, stop, bytes_to_load = 0, start_from_file, back_from_EOF = 0;
	int found_piece = -1, first_file_to_open = 0, last_file_to_open = 0; 
	long long int bytes_to_write = torrent->_piece_length;
	char hash[20];
	SHA1(piece, torrent->_piece_length, hash);
	number_of_pieces = (int) torrent->_hash_length /20;
	for (i = 0; i < number_of_pieces; i++){
		if (strncpy(hash, torrent->_pieces[i]) == 0){
			found_piece = i;
			break;
		}
	}
	if (found_piece == -1){
		fprintf(stderr, "The hash is not found\n");
		return 1;
	}

	start = torrent->_piece_length * found_piece;
	sopt = start + torrent->_piece_length;
	i = 0;
	if (start != 0){
		while (i < torrent->_number_of_files){
			bytes_to_load += torrent->_file_length[i];
			if (bytes_to_load > start){
				back_from_EOF = bytes_to_load - start;
				i--;
				break;
			} else if(bytes_to_load == start){
				back_from_EOF = torrent->_file_length[i];
				break;
			}

		}
	} else {
		//TODO, Add code to handle if it is fist pirece. 
	}
	first_file_to_open = i;

	//Find last_file_to_open. 
	FILE *fp
	fp = fopen(torrent->_file_path[first_file_to_open], "w+");
	fseek(fp, (-back_from_EOF), SEEK_END);
	fprintf(stderr, "current possition in file :%li\n", ftell(fp));
	while(!feof(fp)){
		fprintf(stderr, "%c", piece);
		(char *) piece++;
		bytes_to_write--;
	}
	first_file_to_open++;
	last_file_to_open = first_file_to_open;
	while(bytes_to_write <= 0){
		
	}

	/*while (total_to_load < piece_length && fileindex < torrent->_number_of_files){
    		total_to_load = total_to_load + torrent->_file_length[fileindex];
    		nom_of_files_to_load++;
    		fileindex++;
    }*/

	free (piece);
}