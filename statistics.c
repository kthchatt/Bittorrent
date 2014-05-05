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

Stats loadStats(char *configPath){
	FILE *file = fopen(configPath, "r");
	char *buffer, *tmp;
	int len;
	Stats s;

	if(!file) return s;
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = malloc(fileLen+1);

	fread(buffer, fileLen, 1, file);
	len = strlen(strstr(buffer, ":"));

	tmp = malloc(fileLen-len+1);
	strncpy(tmp, buffer, fileLen-len);
	s.uploaded = atoi(tmp);

	for(i=0; i<len-1; i++)
		tmp[i] = buffer[i+fileLen-len+1];
	s.downloaded = atoi(tmp);

	free(tmp);
	return s;
}