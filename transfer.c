#include "transfer.h"

int prepareFile(char *filePath, int fileSize){
	int i;
	FILE *file = fopen(filePath, "wb");
	if(!file) return 0;

	for(i=0; i<fileSize; i++)
		fwrite('0', 1, sizeof('0'), file); // write each byte to file

	fclose(file);
	return 1;
}

int recievePiece(char *filePath, unsigned char *pieceHash, int pieceSize, int pieceIndex){
	struct sockaddr_in client, server;
	int s, i, slen=sizeof(server);
	unsigned char buffer[pieceSize], hash[SHA_DIGEST_LENGTH];

	FILE *file = fopen(filePath, "wb");
	if(!file) return 0;

	fseek(file, pieceSize*pieceIndex, SEEK_SET); // Set file pointer to the place our piece belongs

	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) return 0;

	memset((char *) &client, 0, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, &client, sizeof(client))==-1) return 0;

	if(recvfrom(s, buffer, pieceSize, 0, &server, &slen)==-1) return 0;	// Receive piece and write to buffer

	SHA1(buffer, sizeof(buffer), hash);
	for(i=0; i<SHA_DIGEST_LENGTH; i++)
		if(hash[i] != pieceHash[i]) return -1;	// check if hashes match

	fwrite(buffer, sizeof(buffer), 1, file); // Write piece to file
	fclose(file);

	return 1;
}

int sendPiece(char *filePath, char *destIP, int pieceSize, int pieceIndex){
	struct sockaddr_in server;
	int s, slen=sizeof(server);
	unsigned char buffer[pieceSize];

	FILE *file = fopen(filePath, "rb");
	if(!file) return 0;
	fseek(file, pieceSize*pieceIndex, SEEK_SET); // Set file pointer to requested piece
	fread(&buffer, pieceSize, 1, file);	// Read piece from file into buffer

	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) return 0;

	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if(inet_aton(destIP, &server.sin_addr)==0) return 0;
	if(sendto(s, buffer, pieceSize, 0, &server, slen)==-1) return 0; // Send piece to server

	fclose(file);
	close(s);
	return 1;
}
