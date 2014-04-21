#include "bencodning.h"
#include <stdio.h>

int main (int argc, char *argv[]) {
	torrent_info data;
	fprintf(stderr, "This is main\n");
	decode_bencode(argv[1], &data);
	fprintf(stderr, "Decode is done\n");
	fprintf(stderr, "Print from main: %s\n", data._announce);
	return 0;
}