#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct RSS{
	char *title;
}RSS;


int getFeed(char *destAddr, char *dir){
	struct hostent *server;
	struct sockaddr_in serverAddr;
	int s, slen=sizeof(server);
	char *request;
	char buffer[900];

	request = malloc(sizeof(dir)+sizeof(destAddr)+31*sizeof(char));
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", dir, destAddr);

	if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1) return 0;

	server = gethostbyname(destAddr);
	if(server==NULL) return 0;
	bzero((char *) &serverAddr.sin_addr.s_addr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(80);

	if(connect(s, (struct sockaddr*) &serverAddr, sizeof(serverAddr))<0) perror("Connect : ");
	if(send(s, request, strlen(request), 0)<strlen(request)) perror("Send : ");

	if(recv(s, buffer, 900, 0)==0){
		perror("Recv : ");
	}
	puts(buffer);
	close(s);
	return 1;
}


int main(){
	getFeed("showrss.info", "/feeds/27.rss");
	return 1;
}