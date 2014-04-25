#include "bencodning.h"
#include <stdio.h>

int main (int argc, char *argv[]) {
	int i = 0;
	torrent_info data;
	fprintf(stderr, "This is main\n");
	decode_bencode(argv[1], &data);
	fprintf(stderr, "Decode is done\n");
	fprintf(stderr, "Print from main: %s\n", data._announce);
	if(data._multi_file == 1){
		printf("This is a multi file torrent, number of files: %d\n", data._number_of_files);
		for (i = 0; i < ((data._number_of_files)/2); ++i)
		{
			fprintf(stderr, "File %d: %55s \t Length: %lli \n", i, data._file_path[i], data._file_length[i]);
		
		}
	} else {
		printf("This is a singel file torrent\n");
	}
	return 0;
}