#include "netstat.h"

/* netstat.c
 * 2014-05-08
 * Robin Duda
 *  Tracks network rates per info_hash.
 *  Todo: add formatting for B/s, kB/s, mB/s, gB/s, as float.
 *  Todo: add total transfer rate.
 *  Todo: add total bytes transferred, store in stats?
 * calculates the average byte rates over the last DELTA_TIME and waits for another DELTA_TIME
 * until refreshing. This could be improved to be more responsive and less fluctuative, by using the latest
 * values which occured the last Xs and update every X/10s.
 */


typedef struct 
{
	unsigned int in_delta, out_delta, in, out;
	char* info_hash;
	pthread_mutex_t statlock;
} netstat_t;

netstat_t* netstat;
pthread_t timer;
pthread_mutex_t binlock;
int tracking_count;


void* timer_thread(void* arg)
{
	int i;

	while (true)
	{
		usleep(DELTA_SAMPLE * 1000);

		for (i = 0; i < tracking_count; i++)
		{
			lock(&netstat[i].statlock);
			netstat[i].in = (netstat[i].in_delta / ((float) DELTA_SAMPLE / 1000));
			netstat[i].out = (netstat[i].out_delta / ((float) DELTA_SAMPLE / 1000));
			netstat[i].in_delta = 0;
			netstat[i].out_delta = 0;
			unlock(&netstat[i].statlock);
			printf("\n%d was reset to %d", i, netstat[i].in_delta);
		}
	}
}

//calling init will start the timer thread.
void netstat_init()
{
	tracking_count = 0;
	netstat = (netstat_t*) malloc(sizeof(netstat_t));	//get a pointer to any block of memory, for realloc. lol.

	if (!(pthread_create(&timer, NULL, timer_thread, NULL)))
		printf("\nTracking your network statistics.");
}

//not safe. safe enough?
void netstat_track(char* info_hash)
{
	int i;
	bool tracked = false;

	for (i = 0; i < tracking_count; i++)
		if (netstat[i].info_hash == info_hash)
		{
			tracked = true;
			printf("\nError: Already Tracking Info Hash %s", info_hash);
		}

	if (tracked == false)
	{
		if (realloc(netstat, sizeof(netstat_t) * tracking_count + 1) != NULL)
		{
			memset(&netstat[tracking_count], 0, sizeof(netstat_t) * tracking_count + 1);
			pthread_mutex_init(&netstat[tracking_count].statlock, NULL); 
			netstat[tracking_count].info_hash = info_hash;
			tracking_count++;
		}
		else
			tracking_count--;
	}
}

void netstat_update(int direction, int amount, char* info_hash)
{
	int i;

	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			lock(&netstat[i].statlock);
			switch (direction)
			{
				case INPUT:  netstat[i].in_delta += amount; break;
				case OUTPUT: netstat[i].out_delta += amount; break;
			}
			unlock(&netstat[i].statlock);
		}
	}
}

int netstat_bytes(int direction, char* info_hash)
{
	int i, amount;

	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			lock(&netstat[i].statlock);
			switch (direction)
			{
				case INPUT:  amount = netstat[i].in;  break;
				case OUTPUT: amount = netstat[i].out; break;
			}
			unlock(&netstat[i].statlock);
		}
	}
	return amount;
}

char* netstat_formatbytes(int direction, char* info_hash)
{
	char* format_string = (char*) malloc(10);
	int rate = netstat_bytes(direction, info_hash);

	memset(format_string, '\0', 10);
	sprintf(format_string, "%d kB/s", rate);

	return format_string;
}

void main(void)
{
	netstat_init();
	netstat_track("oakamfkajfoakdjaks20");

	while (true)
	{
		usleep(100000);

		netstat_update(INPUT, rand()%30+20, "oakamfkajfoakdjaks20");
		netstat_update(OUTPUT, rand()%49+5, "oakamfkajfoakdjaks20");
		printf("\nINPUT: %s", netstat_formatbytes(INPUT, "oakamfkajfoakdjaks20"));
		printf("\nOUTPUT: %s", netstat_formatbytes(OUTPUT, "oakamfkajfoakdjaks20"));
		printf("\n-------------------");
	}
}