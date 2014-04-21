/*
This is Bencodning lib.
Wotking status:		Under construction

The sole purpuse of this lib is to decode bencodning.
*/
//#include "bencodning.h"

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

	long long int _piece_length;
	int _private;

	char _file_name [250];
	long long int _file_length;
	char _file_MD5 [32];

	long int _hash_length;
	char _pieces [5000][20];
} torrent_info;

int init_file (char *, FILE *);
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

int main(int argc, char *argv[]){
	torrent_info data;
	char bencode[25000];
	char string_name[250];
	char file_name[250] = "torrent.torrent";
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

//TODO denna behöver köras flera gånger. 
	test_dictonary_list = read_one_char(fp);
	switch(test_dictonary_list){
		case 'l':
			//fseek(fp,-1, SEEK_CUR);
			list_handler(fp, string_name, &data);
		break;
		case 'd':
			dictonarry_handler(fp, &data, string_name);
		break;
		case 'i':

		break;
		default:
			fprintf(stderr, "Next\n");
		break;
	}
	//fprintf(stderr, "Entire file is read\n");
	//char dataptr = data._pieces[0][0];
	for (i = 0; i < 50; ++i)
	{
		for (j = 0; j < 20; ++j)
		{
			fprintf(stderr, "%02x", (unsigned char) data._pieces[i][j]);
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
	

	//close(fp);
}

int init_file (char *name, FILE *fp){
}

void list_handler(FILE *sfp, char *string_name, torrent_info *data){
	char one_char;
	char string_value[250];
	int length_of_next_int = 0;
	while((one_char = read_one_char(sfp)) != 'e'){
		//fprintf(stderr, "One char from list_handler %c\n", one_char);
		if (one_char == 'l'){
			list_handler(sfp, string_name, data);
		} else {
			fseek(sfp,-1, SEEK_CUR);
		}
		length_of_next_int = read_length_of_next(sfp);
		read_specific_length(sfp, length_of_next_int, string_value);
		if (length_of_next_int == 0){
			fseek(sfp,-1, SEEK_CUR);
			//break;
		}
		
	}
	fprintf(stderr, "%s = %s\n", string_name, string_value);
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
					continue;
				break;
				case 'i':
					int_handler(sfp, string_name, data);
					continue;
				break;
				default:
					fprintf(stderr, "Next\n");
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
		data->_file_length = atoll(string_value);
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

