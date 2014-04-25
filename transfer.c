#include "transfer.h"

int recievePiece(char *filePath, int pieceSize, int pieceIndex){
	struct sockaddr_in client, server;
	int s, i, slen=sizeof(server);
	char buffer[pieceSize];

	FILE *file = fopen(filePath, "wb");	
	if(!file) return 0;

	fseek(file, pieceSize*pieceIndex, SEEK_SET); // Set file pointer to the place our piece belongs

	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) return 0; 

	memset((char *) &client, ,0, sizeof(client));
	client->sin_family = AF_INET;
	client->sin_port = htons(PORT);
	client->sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, &client, sizeof(client))==-1) return 0;	

	if(recvfrom(s, buffer, pieceSize, 0, &server, &slen)==-1) return 0;	// Recieve piece and write to buffer

	fwrite(buffer, sizeof(buffer), 1, file); // Write piece to file
	fclose(fwrite);

	return 1;
}
