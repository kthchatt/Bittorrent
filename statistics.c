#include "statisticts.h"

int saveStats(char *configPath, Stats s){
	FILE *file = fopen(configPath, "w");
	char str[strlen(s.uploaded)+strlen(s.downloaded)+2];
	if(!file) return -1;

	sprintf(str, "%s:%s", s.uploaded, s.downloaded);
	fwrite(str, 1, sizeof(str), file);

	fclose(file);
	return 0;
}