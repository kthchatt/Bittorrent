#include "statisticts.h"

int saveStats(char *configPath, int uploaded, int downloaded){
	FILE *file = fopen(configPath, "w");
	char str[strlen(uploaded)+strlen(downloaded)+2];
	if(!file) return -1;

	sprintf(str, "%s:%s", uploaded, downloaded);
	fwrite(str, 1, sizeof(str), file);

	fclose(file);
	return 0;
}