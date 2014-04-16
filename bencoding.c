/*
This is Bencodning lib.
Wotking status:		Under construction

The sole purpuse of this lib is to decode bencodning.
*/
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
} torrent_info;

int init_file (char *, FILE *);
char read_one_char (FILE *);
void read_specific_length (FILE *, int, char *);
int read_length_of_next (FILE *);
int dictonaty_encoding (FILE *);
void complete_dictonarry (char *, char *, torrent_info *);
void dictonarry_handler (FILE *, torrent_info *);


int main(){
	torrent_info data;
	char bencode[25000];
	char file_name[] = "torrent.torrent";
	char test_dictonary_list;
	int i = 0; int length_of_next_int = 0;
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

		break;
		case 'd':
			dictonarry_handler(fp, &data);
		break;
		case 'i':

		break;
		default:
			fprintf(stderr, "Next\n");
		break;
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
/*Dictonarry handler tar hand om när det kommer en ordlista 
(beydelser), Den tar bara hand om hela betydelser. Betydelser 
som består av ordlistor måste hanteras i separata funktioner. 
*/
void dictonarry_handler (FILE *sfp, torrent_info *data){
	int i = 0; int length_of_next_int = 0;
	int not_complete_dictonarry = 0;
	char string_name[250];
	char string_value[250];
	while(1){
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR); //spolar tillbaka filperkaren ett steg.
			break;
		}
		read_specific_length(sfp, length_of_next_int, string_name);
		length_of_next_int = read_length_of_next(sfp);
		if (length_of_next_int == 0){
			not_complete_dictonarry = 1;
			fseek(sfp,-1, SEEK_CUR);
			break;
		}
		read_specific_length(sfp, length_of_next_int, string_value);
		complete_dictonarry(string_name, string_value, data);

	}

}

void announce_list_handler(){
	//TODO Här skall det läggas till en fuktion för hantera 
	//announce list. Det skall lagras i samma struct som allt annat.

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
	}
	fprintf(stderr, "%s = %s\n", string_name, string_value);

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
	while (last_char != ':' && i < 20){
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

