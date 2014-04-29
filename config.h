#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>

typedef struct Config{
	char nameEx;
	int example;
}Config;

int saveConfig(char *filePath, Config *conf);
Config loadConfig(char *filePath);


#endif
