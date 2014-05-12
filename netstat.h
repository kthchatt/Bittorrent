#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define bool char
#define true 1
#define false 0
#define INPUT 1
#define OUTPUT 2
#define ENABLE 1
#define DISABLE 0
#define U_NONE 0
#define U_BYTE 1
#define U_KILO 1000
#define U_MEGA 1000000
#define BYTE "B/s"
#define KILO "KB/s"
#define MEGA "MB/s"
#define FORMATSTRING_LEN 25
#define DELTA_SAMPLE 325
#define lock pthread_mutex_lock
#define unlock pthread_mutex_unlock

//it is most severe that you run networkstats_init, it is the keeper of time!
void netstat_initialize();
//Set up tracking for an info_hash, it is most beneficial to call this.
void netstat_track(char* info_hash);
//Stop tracking an info_hash. Call on torrent stop.
void netstat_untrack(char* info_hash);
//add bytes of a current to be tracked, either in or out.
void netstat_update(int direction, int amount, char* info_hash);
//get the total throughput of all transfers.
char* netstat_throughput(int direction, char* format);
//returns the decimal value of the amount of bytes received per second average.
int netstat_bytes(int direction, char* info_hash);
//will return stats formatted, "kB/s", "mB/s", "B/s", "gB/s". IMPORTANT: free on reentrance. (todo require an already allocated pointer.)
char* netstat_formatbytes(int direction, char* info_hash, char* format_string);

//an example call to networkstats_formatbytes, used for updating the networking rates.
/*
	void some_main_stuff(...)
	{
		char* mytext = "";

		while (downloading)
		{
			free(mytext);
			mytext = networkstats_formatbytes(OUTPUT, "info_hash_is_20_bytes.");
		}
		free(mytext);
	}

	in another thread..
	num = recv(..., ..., ...);
	netstat_update(INPUT, num, swarm->info_hash);

*/