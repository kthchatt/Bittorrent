#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "announce.h"

int main(int argc, char ** argv)
{
	char* URL = "http://www.google.com:541/announce";

//int tracker_announce(char* tracker, char* info_hash, char* peer_id, char* ip, char* event, int downloaded, int left);
	tracker_announce(URL, "info_hash", "peer_id", "10.0.0.0", "Started", 0, 123918);
}