//TODO !!! Open file but do not overwrite 
#include "createfile.h"
double create_file_status(torrent_info *torrent){
	torrent_info *torrentptr;

	if(torrent != NULL){
		torrentptr = torrent;
	}
	if (torrentptr != NULL){
		return ((double) torrentptr->_total_created / (double) torrentptr->_total_length);
	} 
	return 0;
}

void *create_file(void *ptr){
	torrent_info *torrent = ptr; 
	//fprintf(stderr, "Create File is running \n");
	int i;
	int length;
	long long int total_length = 0;
	long long int total_created = 0;
	int create = 0;
	char tmp_path[500];
	char tmp_string[500];
	char working_dir[200];
	getcwd(working_dir, 200);
	//fprintf(stderr, "Test 1\n");
	//fprintf(stderr, "Current working directory is :%s\n", working_dir);

	for (i = 0; i < torrent->_number_of_files; i++)
	{
		total_length += torrent->_file_length[i];
	}
	torrent->_total_length = total_length;
	for (i = 0; i < torrent->_number_of_files; i++){
		//fprintf(stderr, "TEST\n");
		FILE *fp;
		memset(tmp_path, '\0', 500);
		strcat(tmp_path, working_dir);
		strcat(tmp_path, "/");
		strcat(tmp_path, torrent->_torrent_file_name);

		char *start = torrent->_file_path[i];
		char *filename = strrchr(torrent->_file_path[i], '/');
		length = filename - start;
		if (filename == NULL && torrent->_number_of_files <= 1){
			filename = torrent->_file_name;
		} else if (filename == NULL){
			filename = torrent->_file_path[i];
		}
		while (*filename == '/'){
			filename++;
		}
		//Path
		strncat(tmp_path, torrent->_file_path[i], length);
		strcpy(tmp_string, "mkdir -p ");
		char *ptr = tmp_path;
		strcat(tmp_string, "\"");
		strcat(tmp_string, tmp_path);
		strcat(tmp_string, "\"");
		system(tmp_string);
		//Filename
		strcat(tmp_path, "/");
		strcat(tmp_path, filename);
		//TODO Comment out foloowing line:
		//fprintf(stderr, "%s\n", tmp_path);

		fp = fopen(tmp_path, "rb+");
		if(fp == NULL){
			fp = fopen(tmp_path, "wb+");
			if (fp == NULL){
				fprintf(stderr, "ERROR CREATING FILE\n");
				return ptr;
			}
		} else {
			//fprintf(stderr, "File already exist\n");
		}

		

		total_created = 0;
		while ((total_created+TO_CREATE) < torrent->_file_length[i]){
			fseek(fp, TO_CREATE, SEEK_CUR);
			torrent->_total_created += TO_CREATE;
			total_created += TO_CREATE;
		}
		create = torrent->_file_length[i] -total_created;
		fseek(fp, create -1, SEEK_CUR);		//file size not one byte too large anymore. ~RD
		torrent->_total_created += create;
		fputc('\n', fp);					//critical error: file size will be one byte too, fputc writes past "create" fixed L@89 ~RD
		fclose(fp);

		//fprintf(stderr, "File is set to size\n");
		strcpy(torrent->_file_path[i], tmp_path);
		//fprintf(stderr, "Round nr: %d is donne\n", i);
	}
	/*free (tmp_path);
	free (tmp_string);
	free (working_dir);*/
	return NULL;


}

