#include "pieces.h"

int main()
{
	// example
    unsigned char *test = getHash("t.torrent", 2, 20); // get first hash, second hash = 1 etc
    printf("%d", findHash(test, "t.torrent", 20));
	return 0;
}

int findHash(unsigned char hash[], char filePath[], int totalPieces){
    FILE *file;
	unsigned char *buffer, *result;
	unsigned long fileLen;
	int counter=0;
	unsigned char tmpHash[20];
	strcpy(tmpHash, hash);
	//Open file

	file = fopen(filePath, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", filePath);
		return -1;
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

    // find start of hash table
    result = strstr(buffer, ":pieces");

    int i, offset=1;
    for(;;){
        if(result[offset]==':') break;
        offset++;
    }

    while(counter < totalPieces){
        for(i=0; i<20; i++){
            if(tmpHash[i]!=result[offset+i+1+(counter*20)]) break;
            if(i==19) return counter;
        }
        counter++;
    }
    free(buffer);
    return 0;
}

unsigned char *getHash(char torrentPath[], int piece, int totalPieces){
    FILE *file;
	unsigned char *buffer, *result;
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

    // find start of hash table
    result = strstr(buffer, ":pieces");

    // find offset amount
    int i, offset=1;
    for(;;){
        if(result[offset]==':') break;
        offset++;
    }

    for(i=0; i<totalPieces; i++){
        hash[i]=result[offset+i+1+(piece*20)];
    }
	free(buffer);
	return &hash;
}
