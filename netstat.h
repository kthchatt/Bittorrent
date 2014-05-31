/* netstat.c
 * 2014-05-08
 * Robin Duda 
 *
 * Track network rates, up/down - per torrent and up/down total.
 */

 #ifndef _netstat_h
 #define _netstat_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> 
#include "protocol_meta.h"

#define bool char
#define true 1
#define false 0
#define INPUT 1
#define OUTPUT 2
#define ENABLE 1
#define DISABLE 0
#define DELTA_SAMPLE 1000
#define lock pthread_mutex_lock
#define unlock pthread_mutex_unlock
#define RATIO_PRECISION "[%.10f]"

void netstat_initialize();												//initialize it. it is the keeper of time!
void netstat_track(char* info_hash, unsigned long int filesize);		//Set up tracking for an info_hash, it is most beneficial to call this.
void netstat_untrack(char* info_hash);									//Stop tracking an info_hash. Call on torrent stop.
void netstat_update(char* info_hash, int direction, int amount);	//add bytes of a current to be tracked, either in or out.
char* netstat_throughput(int direction, char* format);				//get the total throughput of all transfers.
int   netstat_bytes(char* info_hash, int direction);				//returns the decimal value of the amount of bytes received per second average.
char* netstat_formatbytes(char* info_hash, int direction, char* format_string); //returns formatted net-rates, "kB/s", "mB/s", "B/s", "gB/s".
char* netstat_formatratio(char* info_hash, char* format);						//returns a formated ratio.
void  netstat_ratio(char* info_hash, int amount);								//update the ratio.

#endif