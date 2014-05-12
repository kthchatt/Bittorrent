#include "netstat.h"

/* netstat.c
 * 2014-05-08
 * Robin Duda
 *  Tracks network rates per info_hash.
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
int tracking_count;
static bool netstat_initialized = false;


void* timer_thread(void* arg)
{
	int i;

	char* damoss = (char*) malloc(FORMATSTRING_LEN); //remove

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
			printf("\nDownload %s", netstat_formatbytes(INPUT, netstat[i].info_hash, damoss)); 
			printf("\tUpload %s",   netstat_formatbytes(OUTPUT, netstat[i].info_hash, damoss));
		}
	}
	free(damoss);
}

//calling init will start the timer thread.
void netstat_initialize()
{
	if (netstat_initialized == false)
	{
		tracking_count = 0;
		//netstat = (netstat_t*) malloc(sizeof(netstat_t));	//get a pointer to any block of memory, for realloc. lol.
		//memset(netstat, 0, sizeof(netstat_t));

		if (!(pthread_create(&timer, NULL, timer_thread, NULL)))
			printf("\nTracking your network statistics.");

		netstat_initialized = true;
	}
}

//not safe. safe enough?
void netstat_track(char* info_hash)
{
	int i;
	bool tracked = false;

	for (i = 0; i < tracking_count; i++)
		if (netstat[i].info_hash == info_hash)
			tracked = true;

	if (tracked == false)
	{
		if ((netstat = realloc(netstat, sizeof(netstat_t) * (tracking_count + 1))) != NULL)
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
		if (*netstat[i].info_hash == *info_hash)
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
		if (*netstat[i].info_hash == *info_hash)
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

//passing a pointer as argument removes the need for freeing and mallocing every call.
//returning the same pointer allows for function inline.
char* netstat_formatbytes(int direction, char* info_hash, char* format_string)
{
	float rate = netstat_bytes(direction, info_hash);
	int unit;
	memset(format_string, '\0', FORMATSTRING_LEN);

	if (rate < U_BYTE)
	{
		strcat(format_string, "N/A");
		return format_string;
	}

	if (U_NONE < rate  && rate < U_KILO)  unit = U_BYTE;
	if (U_KILO <= rate && rate < U_MEGA) unit = U_KILO;
	if (rate >= U_MEGA) 				 unit = U_MEGA;

	rate = (rate/unit);
	sprintf(format_string, "%.2f ", rate);

	switch (unit)
	{
		case U_BYTE: strcat(format_string, BYTE); break;
		case U_KILO: strcat(format_string, KILO); break;
		case U_MEGA: strcat(format_string, MEGA); break;
	}
	return format_string;
}

/*void main(void)
{
	char* format_string = (char*) malloc(FORMATSTRING_LEN);
	netstat_init();
	netstat_track("oakamfkajfoakdjaks20");
	int byteader = 0;

	while (true)
	{
		usleep(100000);
		byteader += 24;
		netstat_update(INPUT, rand()%30+20+byteader, "oakamfkajfoakdjaks20");
		netstat_update(OUTPUT, rand()%49+5+byteader, "oakamfkajfoakdjaks20");
		printf("\nINPUT: %s", netstat_formatbytes(INPUT, "oakamfkajfoakdjaks20", format_string));
		printf("\nOUTPUT: %s", netstat_formatbytes(OUTPUT, "oakamfkajfoakdjaks20", format_string));
		printf("\n-------------------");
	}
}*/