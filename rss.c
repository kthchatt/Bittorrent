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
	char request[];
	char buffer[100];

	request = malloc(sizeof(dir)+sizeof(destAddr)+16*sizeof(char));
	sprinf(request, "GET%cHTTP/1.1Host:%s", dir, destAddr);

	if((s=socket(AF_INET, SOCK_STREAM, 0))==-1) return 0;

	server = gethostsbyname(destAddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(80);

	if(connect(s, &serverAddr, sizeof(serverAddr))<0) return 0;
	if(send(s, request, sizeof(request), 0)<0) return 0;

	while(recv(s, buffer, 100, 0)){
		puts(buffer);
	}
	close(s);
	return 1;
}


int main(){
	getFeed("showrss.info", "/feeds/27.rss");
	return 1;
}