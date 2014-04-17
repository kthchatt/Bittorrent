#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//example call to tracker "announce".
/*announce("protocol://retracker.hq.ertelecom.ru:port/announce-url",	//URL TO TRACKER
	"iiiinnnnffffoooohashkkkkk", 										//METAINFO HASH
	"peerIDaaaabbbbcdeeff", 											//PEERID
	"0", 																//IP
	"Started",															//EVENT [started, stopped, completed] 
	0, 																	//DOWNLOADED
	120582);															//LEFT
*/

//TODO: announce must return a parsed peerlist.

int announce(char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 

#endif