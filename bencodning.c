/*


This is Bencodning lib.
Wotking status:		Under construction

The sole purpuse of this lib is to decode bencodning.
*/

#include "bencodning.h"

//Global var
int place_announce_list = 0;
int place_files = 0;

int hashncmp (unsigned char *hash1, unsigned char *hash2, int n){
	int i;
	unsigned char *ptr1 = hash1;
	unsigned char *ptr2 = hash2;

	for (i = 0; i < n; i++){
		if (*ptr1 != *ptr2){
			return 1;
		} else {
			ptr1++;
			ptr2++;
		}
	}
	return 0;
}

int decode_bencode(char *file_name, torrent_info *data){
	place_files = 0;
	place_announce_list = 0;
	//fprintf(stderr, "This is print from decode_bencode\n");
	//torrent_info data;
	//char bencode[25000];
	char string_name[250];
	//char file_name[250] = "torrent.torrent";
	char test_dictonary_list;
	char *to_null;
	int j = 0;
	//fprintf(stderr, "decode_bencode got file: %s \n", file_name);

	FILE *fp;
	fp = fopen(file_name, "r");
	if (fp != NULL){
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
	
	to_null = strrchr(file_name, '.');
	*to_null = '\0';
	strcpy(data->_torrent_file_name, file_name);

	//fprintf(stderr, "Calculating hash.....\n:");
	info_hash(fp, data);
	//fprintf(stderr, "The info hash is:");
	for (j = 0; j < 20; ++j)
		{
			fprintf(stderr, "%02x", (unsigned char) data->_info_hash[j]);
			//dataptr++;
		}
	//fprintf(stderr, "\n");
	if (place_announce_list < 16)
	    data->_announce_list_count = place_announce_list;
	else
		data->_announce_list_count = 16;
	return 1;

}

//Calculates the info hash.
void info_hash (FILE *sfp, torrent_info *data){
	int end = fseek(sfp, 0, SEEK_END);
	end = ftell(sfp) - 1;
	int toread = end - data->_location;
	//fprintf(stderr, "End = %d Start = %ld To Read = %d\n", end, data->_location, toread);
	unsigned char *indata = (unsigned char *) malloc(toread);
	fseek(sfp, data->_location, SEEK_SET);
	//fprintf(stderr, "The current possition is: %ld \n", ftell(sfp));
	fread(indata, 1, toread, sfp);
	//fprintf(stderr, "Read is done \n");
	const char hash[21];
	//fprintf(stderr, "Attempting to hash \n");
	SHA1(indata, toread, (unsigned char *)hash);
	//fprintf(stderr, "Hash is done\n");
	strncpy(data->_info_hash, hash, 20);
	//fprintf(stderr, "String is copied\n");
	free(indata);
	return;
}


void path_handler(char *string_value, int ready_to_store, torrent_info *data){
	static char file_path[500] = "\0";
	if ( ready_to_store == 1){
		if(strcmp(file_path, "") == 0){
			return;
		}
		complete_dictonarry("path", file_path, data);
		file_path[0] = '\0';
		data->_multi_file = 1;
		return;
	}
	strcat(file_path, "/");	
	strcat(file_path, string_value);
	return;
}

void list_handler(FILE *sfp, char *string_name, torrent_info *data){
	char one_char;
	char string_value[500];
	memset(string_value, 0, 500);
	//char file_path[500];
	int length_of_next_int = 0;	
	while((one_char = read_one_char(sfp)) != 'e'){
		//fprintf(stderr, "One char from list_handler %c\n", one_char);
		if (one_char == 'l'){
			list_handler(sfp, string_name, data);
		} else if (one_char == 'd') {
			dictonarry_handler(sfp, data, "kakor");
			continue;
		} else if(one_char == 'i'){
			int_handler(sfp, string_name, data);
		} else {
			fseek(sfp,-1, SEEK_CUR);
		}
		length_of_next_int = read_length_of_next(sfp);
		read_specific_length(sfp, length_of_next_int, string_value);
		if (strcmp(string_name, "path") == 0){
			path_handler(string_value, 0, data);
		} else if (string_value[0] != '\0') { //TODO Den kör denna även när den inte skall, kolla string_value
			complete_dictonarry(string_name, string_value, data);
		}
		if (length_of_next_int == 0){
			fseek(sfp,-1, SEEK_CUR);
		}
		
	}
	return;
}
/*Dictonarry handler tar hand om när det kommer en ordlista 
(beydelser), Den tar bara hand om hela betydelser. Betydelser 
som består av ordlistor måste hanteras i separata funktioner. 
*/
void dictonarry_handler (FILE *sfp, torrent_info *data, char *string){
	int length_of_next_int = 0;
	//int not_complete_dictonarry = 0;
	char string_name[250];
	char string_value[250];
	char one_char;
	while(1){
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			//not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR); //spolar tillbaka filperkaren ett steg.
			one_char = read_one_char(sfp);
			switch(one_char){
				case 'l':
					list_handler(sfp, string_name, data);
					path_handler("kakor", 1, data);
					continue;
				break;
				case 'd':
					continue;
				break;
				case 'i':
					int_handler(sfp, string_name, data);
					continue;
				break;
				case 'e':
					return;
				break;
				default:
					fprintf(stderr, "MIDDLE OF STRING ERROR FROM FIRST dictonarry_handler, last char was: %c\n", one_char);
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
		} 
		if (strcmp("info", string_name) == 0){
			data->_location = ftell(sfp);
			//fprintf(stderr, "We got: %s at possition %ld \n", string_name, data->_location);
		}
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			//not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR);
			one_char = read_one_char(sfp);
			switch(one_char){
				case 'l':
					list_handler(sfp, string_name, data);
					path_handler("kakor", 1, data);
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
	//unsigned char hash[20];
	//int i = 0; int j = 0; int k = 0;
	int length_of_next_int = 0;
	//unsigned char test[20000];
	length_of_next_int = read_length_of_next(sfp);
	data->_hash_length = length_of_next_int;
	//char *picesptr = &data->_pieces[0][0];
	//fprintf(stderr, "Hash handler, hash length = %d\n", length_of_next_int);
	fread(data->_pieces[0], length_of_next_int, 1, sfp);

}

void int_handler(FILE *sfp, char *string_name, torrent_info *data){
	char next_char[20];
	char last_char = '\0'; int i = 0;
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
	//int ascii_int = 0;
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
		if (data->_number_of_files < 1){
			data->_number_of_files = 1;
		}
	} else if (strcmp(string_name, "piece length") == 0)
	{	
		data->_piece_length = atoll(string_value);
	} else if (strcmp(string_name, "announce-list") == 0)
	{	
		strcpy(data->_announce_list[place_announce_list++], string_value);
	} else if (strcmp(string_name, "path") == 0)
	{	
		strcpy(data->_file_path[place_files++], string_value);
		data->_number_of_files = place_files;
	}
	//fprintf(stderr, "%s = %s\n", string_name, string_value);
	return;

}


void read_specific_length (FILE *sfp, int length_of_next_int, char *temporary){
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

