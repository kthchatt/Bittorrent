#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct RSS{
	char title[],
		 description[],
		 link[];
}RSS;


int getFeed(char *destAddr, char *dir){
	struct hostent *server;
	struct sockaddr_in serverAddr;
	int s, slen=sizeof(server);
	char request[] "GET /feeds/27.rss HTTP/1.1\r\nHost: showrss.info\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
	char buffer[100];

	//request = malloc(sizeof(dir)+sizeof(destAddr)+16*sizeof(char));
	//sprintf(request, "GET%cHTTP/1.1Host:%c", dir, destAddr);

	if((s=socket(AF_INET, SOCK_STREAM, 0))==-1) return 0;

	server = gethostsbyname(destAddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(80);

	if(connect(s, (struct serveraddr*) &serverAddr, sizeof(serverAddr))<0) return 0;
	if(send(s, request, sizeof(request), 0)<0) return 0;

	while(recv(s, buffer, 100, 0)){
		puts(buffer);
	}
	puts(buffer);
	close(s);
	return 1;
}


int main(){
	getFeed("showrss.info", "/feeds/27.rss");
	return 1;
}