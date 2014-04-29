#include "config.h"

int saveConfig(char *filePath, Config *conf){
	FILE *file = fopen(filePath, "wb");
	if(!file) return 0;

	//fwrite(conf.nameEx+':', 1, sizeof(conf.nameEx), file);
	//fwrite(conf.example, 1, sizeof(conf.example), file);

	fclose(file);
}

Config loadConfig(char *filePath){
	Config conf;
	FILE *file = fopen (filePath, "rb");

	//conf.nameEx = fread(blabla);
	fclose(file);
	return conf;
}