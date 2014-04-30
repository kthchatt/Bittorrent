#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct Item{
	char *title,
		 *description,
		 *link;
}Item;

int getFeed(char *destAddr, char *dir);
char *getBetweenTags(char *haystack, char *start, char *stop);

Item getFeed(char *destAddr, char *dir){
	Item item;
	struct hostent *server;
	struct sockaddr_in serverAddr;
	int s, slen=sizeof(server);
	char *request;
	char buffer[900];

	request = malloc(sizeof(dir)+sizeof(destAddr)+31*sizeof(char));
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", dir, destAddr);

	if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1) perror("Socket : ");

	server = gethostbyname(destAddr);
	if(server==NULL) return NULL;
	bzero((char *) &serverAddr.sin_addr.s_addr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(80);

	if(connect(s, (struct sockaddr*) &serverAddr, sizeof(serverAddr))<0) perror("Connect : ");
	if(send(s, request, strlen(request), 0)<strlen(request)) perror("Send : ");

	if(recv(s, buffer, 900, 0)==0)
		perror("Recv : ");

	item.title = getBetweenTags(buffer, "<title>", "</title>");
	item.description = getBetweenTags(buffer, "<description>", "</description>");
	item.link = getBetweenTags(buffer, "<link>", "</link>");

	close(s);
	return item;
}

char *getBetweenTags(char *haystack, char *start, char *stop){
	char *tmp  = strstr(haystack, start);
	int c = strlen(tmp) - strlen(strstr(haystack, stop)) - strlen(start);
	int i;
	char *newstr;
	newstr = malloc(sizeof(char)*c);

	for(i=0; i<c; i++)
		newstr[i] = tmp[i+strlen(start)];

	return newstr;
}

int main(){
	Item test = getFeed("showrss.info", "/feeds/27.rss");
	puts(test.title);
	return 1;
}