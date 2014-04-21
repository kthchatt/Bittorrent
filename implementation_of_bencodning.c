#include "bencodning.h"
#include <stdio.h>

int main () {
	torrent_info data;
	fprintf(stderr, "This is main\n");
	decode_bencode("torrent.torrent", &data);
	fprintf(stderr, "Decode is done\n");
	fprintf(stderr, "Print from main: %s\n", data._announce);
	return 0;
}