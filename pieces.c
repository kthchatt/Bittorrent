#include "pieces.h"

int main()
{
	// example
    unsigned char *test = getHash("t.torrent", 0);
    printf("%x", test[0]);
	return 0;
}

unsigned char *getHash(char torrentPath[], int piece){
    FILE *file;
	char *buffer, *result;
	unsigned long fileLen;
	unsigned char hash[20];

	//Open file
	file = fopen(torrentPath, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", torrentPath);
		return hash;
	}
	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(file);
		return hash;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

    // find start of hashes
    result = strstr(buffer, ":pieces");

    // find offset amount
    int i, offset=1;
    for(;;){
        if(result[offset]==':') break;
        offset++;
    }

    for(i=0; i<20; i++){
        hash[i]=result[offset+i+1+(piece*20)];
    }
	free(buffer);
	return &hash;
}
