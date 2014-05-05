#ifndef _statistics_h
#define _statistics_h

#include <string.h>
#include <stdio.h>

typedef struct Stats{
	int uploaded;
	int downloaded;
}Stats;

int saveStats(char *configPath, Stats s);

#endif 