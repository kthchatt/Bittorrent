/* netstat.c
 * 2014-05-08
 * Robin Duda
 *
 *  Tracks network rates per info_hash, only valid data (hash-verified) should be tracked.
 *  	Todo: add total bytes transferred, store in stats?
 *
 * calculates the average byte rates over the last DELTA_TIME and waits for another DELTA_TIME
 * until refreshing. This could be improved to be less fluctuative, sacrificing precision.
 */

#include "netstat.h"

typedef struct 
{
	unsigned int in_delta, out_delta, in, out;
	unsigned long filesize;
	double ratio;
	char* info_hash;
	pthread_mutex_t lock;
} netstat_t;

netstat_t* netstat;
netstat_t totalstat;
pthread_t timer;
int tracking_count;
static bool netstat_initialized = false;

//recalculate averages.
void* timer_thread(void* arg)
{
	int i;

	while (true)
	{
		usleep(DELTA_SAMPLE * 1000);

		for (i = 0; i < tracking_count; i++)
		{
			lock(&netstat[i].lock);
			netstat[i].in = (netstat[i].in_delta / ((float) DELTA_SAMPLE / 1000));
			netstat[i].out = (netstat[i].out_delta / ((float) DELTA_SAMPLE / 1000));
			netstat[i].in_delta = 0;
			netstat[i].out_delta = 0;
			unlock(&netstat[i].lock);
		}

		lock(&totalstat.lock);
		totalstat.in = (totalstat.in_delta / ((float) DELTA_SAMPLE / 1000));
		totalstat.out = (totalstat.out_delta / ((float) DELTA_SAMPLE / 1000));
		totalstat.in_delta = 0;
		totalstat.out_delta = 0;
		unlock(&totalstat.lock);
	}
}

//calling init will start the timer thread.
void netstat_initialize()
{
	if (netstat_initialized == false)
	{
		tracking_count = 0;
		totalstat.in_delta = 0;
		totalstat.out_delta = 0;
		totalstat.in = 0;
		totalstat.out = 0;
		pthread_mutex_init(&totalstat.lock, NULL);
		
		netstat = (netstat_t*) malloc(sizeof(netstat_t));	//get a pointer to any block of memory, for realloc. lol.
		memset(netstat, 0, sizeof(netstat_t));

		if (pthread_create(&timer, NULL, timer_thread, NULL))
			printf("\nNetstat initialization failed.");

		netstat_initialized = true;
	}
}

//set up tracking for a specific info_hash.
void netstat_track(char* info_hash, unsigned long int filesize)
{
	int i;
	bool tracked = false;

	for (i = 0; i < tracking_count; i++)
		if (netstat[i].info_hash == info_hash)
			tracked = true;

	if (tracked == false)
	{
		for (i = 0; i < tracking_count; i++)
			lock(&netstat[i].lock);

		if ((netstat = (netstat_t*) realloc(netstat, sizeof(netstat_t) * (tracking_count + 1))) != NULL)
		{
			memset(&netstat[tracking_count], 0, sizeof(netstat_t));
			pthread_mutex_init(&netstat[tracking_count].lock, NULL); 
			netstat[tracking_count].info_hash = info_hash;
			netstat[tracking_count].ratio = 0.0;
			netstat[tracking_count].filesize = filesize;
			tracking_count++;
		}
		else
			tracking_count--;

		for (i = 0; i < tracking_count; i++)
			unlock(&netstat[i].lock);
	}
}

//stop tracking a torrent.
void netstat_untrack(char* info_hash)
{
	int i;

	for (i = 0; i < tracking_count; i++)	//moving the memory, we must lock everything! (could be improved to only lock items which needs to move)
		lock(&netstat[i].lock);

	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			tracking_count--;
			memmove(netstat+i, netstat+i+1, (tracking_count - i) * sizeof(netstat_t));
			netstat = realloc(netstat, sizeof(netstat_t) * tracking_count);
			break;
		}
	}
	for (i = 0; i < tracking_count; i++)
		unlock(&netstat[i].lock);

	return;
}

//format download rate of bytes into something nice. like B/s, KB/s, MB/s, GB/s
char* format_string(char* format_string, float rate)
{
	int unit;
	memset(format_string, '\0', FORMATSTRING_LEN);

	if (rate < U_BYTE)
	{
		strcat(format_string, "N/A");
		return format_string;
	}

	if (U_NONE < rate  && rate < U_KILO) unit = U_BYTE;
	if (U_KILO <= rate && rate < U_MEGA) unit = U_KILO;
	if (rate >= U_MEGA) 				 unit = U_MEGA;

	rate = (rate/unit);
	sprintf(format_string, "%.1f ", rate);

	switch (unit)
	{
		case U_BYTE: strcat(format_string, R_BYTE); break;
		case U_KILO: strcat(format_string, R_KILO); break;
		case U_MEGA: strcat(format_string, R_MEGA); break;
	}

	return format_string;
}

//update the total goodput.
void throughput_update(int direction, int amount)
{
	lock(&totalstat.lock);
	switch (direction)
	{
		case INPUT:  totalstat.in_delta += amount; break;
		case OUTPUT: totalstat.out_delta += amount; break;
	}
	unlock(&totalstat.lock);
}

//update goodput per torrent.
void netstat_update(char* info_hash, int direction, int amount)
{
	int i;
	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			lock(&netstat[i].lock);
			switch (direction)
			{
				case INPUT:  netstat[i].in_delta += amount; break;
				case OUTPUT: netstat[i].out_delta += amount; break;
			}
			unlock(&netstat[i].lock);
		}
	}
	throughput_update(direction, amount);
}

//retrieve the current goodput.
char* netstat_throughput(int direction, char* format)
{
	int rate = 0;

	lock(&totalstat.lock);
	switch (direction)
	{
		case INPUT:  rate =  totalstat.in; break;
		case OUTPUT: rate = totalstat.out; break;
	}
	unlock(&totalstat.lock);

	return format_string(format, rate);
}

//update the down/up- rates.
int netstat_bytes(char* info_hash, int direction)
{
	int i, amount = -1;

	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			lock(&netstat[i].lock);
			switch (direction)
			{
				case INPUT:  amount = netstat[i].in;  break;
				case OUTPUT: amount = netstat[i].out; break;
			}
			unlock(&netstat[i].lock);
		}
	}
	return amount;
}

//returns a formated ratio.
char*  netstat_formatratio(char* info_hash, char* format)						
{
	int i;

	for (i = 0; i < tracking_count; i++)
	{
		if (netstat[i].info_hash == info_hash)
		{
			snprintf(format, FORMATSTRING_LEN, RATIO_PRECISION, netstat[i].ratio);
			return format;
		}
	}
	return format;
}

//update the ratio.
void  netstat_ratio(char* info_hash, int amount)								
{
	int i;

	for (i = 0; i < tracking_count; i++)
		if (netstat[i].info_hash == info_hash)
			netstat[i].ratio += ((double) amount / (double) netstat[i].filesize);
}

//format bytes.
char* netstat_formatbytes(char* info_hash, int direction, char* format)
{
	return format_string(format, netstat_bytes(info_hash, direction));
}
