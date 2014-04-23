/*

TODO
In multi file torrent it only reads the first file. This must be addressed.


This is Bencodning lib.
Wotking status:		Under construction

The sole purpuse of this lib is to decode bencodning.
*/

#include "bencodning.h"
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

//Flobal var
int place_announce_list = 0;
int place_files = 0;

int decode_bencode(char *file_name, torrent_info *data){
	//torrent_info data;
	char bencode[25000];
	char string_name[250];
	//char file_name[250] = "torrent.torrent";
	char test_dictonary_list;
	int i = 0; int j = 0; int length_of_next_int = 0;


	FILE *fp;
	fp = fopen(file_name, "r");
	if (fp != NULL){
		fprintf(stderr, "File %s is open\n", file_name);
	} else {
		fprintf(stderr, "Error opening file\n");
		return(-1);
	}

	test_dictonary_list = read_one_char(fp);
	switch(test_dictonary_list){
		case 'l':
			//fseek(fp,-1, SEEK_CUR);
			list_handler(fp, string_name, data);
		break;
		case 'd':
			dictonarry_handler(fp, data, string_name);
		break;
		case 'i':

		break;
		default:
			fprintf(stderr, "Next\n");
		break;
	}
	for (i = 0; i < 50; ++i)
	{
		for (j = 0; j < 20; ++j)
		{
			fprintf(stderr, "%02x", (unsigned char) data->_pieces[i][j]);
			//dataptr++;
		}
		fprintf(stderr, "\n");
	}

	//Skriver ut allt som inte har lästs in än. 
	fgets(bencode, 3000, fp);
	for (i = 0; i < 3000; ++i)
	{
		printf("%c", bencode[i]);
	}
	return 1;

}

int init_file (char *name, FILE *fp){
}

//TODO Files handler. 
/*
void files_handler (FILE *sfp, torrent_info *data){
	int index = 0;
	char file_path [500];
	char string_name[100];
	char one_char;
	int length_of_next_int = 0;
	while (length_of_next_int == 0){
		length_of_next_int = read_length_of_next(sfp);
	}
	one_char = read_one_char(sfp)
	read_specific_length(sfp, length_of_next_int, string_value);
}*/
void path_handler(FILE *sfp, torrent_info *data){
	char one_char = 'a';
	char string_value[250];
	int length_of_next_int = 0;
	char file_path [500] = "\0";
	one_char = read_one_char(sfp);
	while(one_char != 'e'){
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			one_char = read_one_char(sfp);
			if (one_char == 'e'){
				break;
			} else {
				fprintf(stderr, "ERROR MIDDLE OF STRING\n");
			}
		}else {
			strcat(file_path, "/");
		}
		read_specific_length(sfp, length_of_next_int, string_value);
		strcat(file_path, string_value);
		//strcat(file_path, "/");
		fprintf(stderr, "Current status of file path %s\n", string_value);
	}
	strcpy(data->_file_path[place_files++], file_path);
	fprintf(stderr, "File path: %s File length: %lld File number: &d\n", data->_file_path[(place_files-1)], data->_file_length[(place_files-1)], place_files);
	return;

}

void list_handler(FILE *sfp, char *string_name, torrent_info *data){
	char one_char;
	char string_value[250];
	int length_of_next_int = 0;
	if (strcmp(string_name, "announce-list") == 0){
		while((one_char = read_one_char(sfp)) != 'e'){
			//fprintf(stderr, "One char from list_handler %c\n", one_char);
			if (one_char == 'l'){
				list_handler(sfp, string_name, data);
			} else {
				fseek(sfp,-1, SEEK_CUR);
			}
			length_of_next_int = read_length_of_next(sfp);
			read_specific_length(sfp, length_of_next_int, string_value);
			strcpy(data->_announce_list[place_announce_list++], string_value);
			if (length_of_next_int == 0){
				fseek(sfp,-1, SEEK_CUR);
				//break;
			}
			
		}
	} else if (strcmp(string_name, "files") == 0){
		fprintf(stderr, "list handler got %s\n", string_name);
		if ((one_char = read_one_char(sfp)) == 'd' ){
			length_of_next_int = read_length_of_next(sfp);
			read_specific_length(sfp, length_of_next_int, string_value);
			one_char = read_one_char(sfp);
			switch(one_char){
				case 'l':
					list_handler(sfp, string_name, data);
				break;
				/*case 'd':
				break;*/
				case 'i':
					int_handler(sfp, string_value, data);
				break;
				default:
					fseek(sfp,-1, SEEK_CUR);
					//fprintf(stderr, "Next\n");
					return;
				break;
			}
			length_of_next_int = read_length_of_next(sfp);
			if (length_of_next_int == 0){
			} else {
				read_specific_length(sfp, length_of_next_int, string_value);
			}
			if (strcmp(string_value, "path") == 0){
				path_handler(sfp, data);
				one_char
			}
		}
	}
	fprintf(stderr, "%s = %s\n", string_name, data->_announce_list[(place_announce_list-1)]);
	return;
}
/*Dictonarry handler tar hand om när det kommer en ordlista 
(beydelser), Den tar bara hand om hela betydelser. Betydelser 
som består av ordlistor måste hanteras i separata funktioner. 
*/
void dictonarry_handler (FILE *sfp, torrent_info *data, char *string){
	int i = 0; int length_of_next_int = 0;
	int not_complete_dictonarry = 0;
	char string_name[250];
	char string_value[250];
	char one_char;
	while(1){
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR); //spolar tillbaka filperkaren ett steg.
			one_char = read_one_char(sfp);
			switch(one_char){
				case 'l':
					list_handler(sfp, string_name, data);
					continue;
				break;
				case 'd':
					break;
					continue;
				break;
				case 'i':
					int_handler(sfp, string_name, data);
					continue;
				break;
				default:
					fprintf(stderr, "MIDDLE OF STRING ERROR\n");
					return;
				break;
			}
			break;
			//return string_name;
		}
		read_specific_length(sfp, length_of_next_int, string_name);
		if(strcmp(string_name, "pieces") == 0){
			hash_handler(sfp, data);
			break;
		} else if  (strcmp(string_name, "files") == 0){
			fprintf(stderr, "WE GOT FILES\n");
			//continue;
		}
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR);
			one_char = read_one_char(sfp);
			switch(one_char){
				case 'l':
					list_handler(sfp, string_name, data);
					continue;
				break;
				case 'd':
					continue;
				break;
				case 'i':
					int_handler(sfp, string_name, data);
					continue;
				break;
				default:
					return;
				break;
			}
			break;
		}
		read_specific_length(sfp, length_of_next_int, string_value);
		complete_dictonarry(string_name, string_value, data);

	}
	return;

}
void hash_handler(FILE *sfp, torrent_info *data){
	unsigned char hash[20];
	int i = 0; int j = 0; int k = 0;
	int length_of_next_int = 0;
	unsigned char test[20000];
	length_of_next_int = read_length_of_next(sfp);
	data->_hash_length = length_of_next_int;
	char *picesptr = &data->_pieces[0][0];
	fprintf(stderr, "Hash handler, hash length = %d\n", length_of_next_int);
	fread(data->_pieces[0], length_of_next_int, 1, sfp);
	
	/*
	//This section is a backup
	fread(test, length_of_next_int, 1, sfp);
	strcpy(data->_pieces[0], test);

	for (i = 0; i < 5000; ++i){
		for (j = 0; j < 20; ++j)
		{
			data->_pieces[i][j] = test[k++];
		}
	}
	fprintf(stderr, "TESTTEST\n");*/
	
	

}

void int_handler(FILE *sfp, char *string_name, torrent_info *data){
	char next_char[20];
	char last_char = '\0'; int i = 0; int length_of_next_int = 0;
	while (last_char != 'e'){
		fscanf(sfp, "%c", &next_char[i]);
		last_char = next_char[i];
		i++;
	}
	next_char[--i] = '\0';
	complete_dictonarry(string_name, next_char, data);
	return;
}

void announce_list_handler(FILE *sfp, torrent_info *data){
}

void complete_dictonarry (char *string_name, char *string_value, torrent_info *data){
	int ascii_int = 0;
	if (strcmp(string_name, "announce") == 0)
	{	
		strcpy(data->_announce, string_value);
	} else if (strcmp(string_name, "comment") == 0)
	{	
		strcpy(data->_comment, string_value);
	} else if (strcmp(string_name, "creation date") == 0)
	{	
		data->_creation_date = atoll(string_value);
	} else if (strcmp(string_name, "created by") == 0)
	{	
		strcpy(data->_created_by, string_value);
	} else if (strcmp(string_name, "length") == 0)
	{	
		data->_file_length[place_files] = atoll(string_value);
	} else if (strcmp(string_name, "name") == 0)
	{	
		strcpy(data->_file_name, string_value);
	} else if (strcmp(string_name, "pice length") == 0)
	{	
		data->_piece_length = atoll(string_value);
	}
	fprintf(stderr, "%s = %s\n", string_name, string_value);
	return;

}


void read_specific_length (FILE *sfp, int length_of_next_int, char *temporary){
	int i = 0;
	fgets(temporary, (length_of_next_int+1), sfp);
	//printf("\t\t\t\t%s", temporary);
	//printf("\n");
}

int read_length_of_next (FILE *sfp){
	char next_char[20];
	char last_char = '\0'; int i = 0; int length_of_next_int = 0;
	while (last_char != ':' && i < 20 && last_char != 'd' && last_char != 'l' 
				&& last_char != 'i' && last_char != 'e'){
		fscanf(sfp, "%c", &next_char[i]);
		last_char = next_char[i];
		i++;
	}
	next_char[i] = '\0';
	length_of_next_int = atoi (next_char);
	//fprintf(stderr, "Length_of_next = %d\n", length_of_next_int);
	return (length_of_next_int);

}
//läser in en char och retunerar den. 
char read_one_char (FILE *sfp){
	char one_char;
	fscanf(sfp, "%c", &one_char);
	//fprintf(stderr, "One char read is: %c\n", one_char);
	return(one_char);
}


//returns one if it is dictonarry encoded. 
int dictonaty_encoding (FILE *sfp){
	char first_char[1];
	fscanf(sfp, "%c", &first_char[0]);
	if (*first_char == 'd')
	{
		printf("This is a dictonarry encoded file\n");
		return(1);
	} else {
		return(0);
	}

}

