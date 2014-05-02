#include "rss.h"

Items getFeed(char *destAddr, char *dir){
	Item item;
	Items sitems;
	struct hostent *server;
	struct sockaddr_in serverAddr;
	int s, i, counter=0, slen=sizeof(server);
	char *request;
	char buffer[2048], *tmp;

	// allocate memmory for request string
	request = malloc(sizeof(dir)+sizeof(destAddr)+31*sizeof(char));
	// build request
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", dir, destAddr);

	if((s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1) perror("Socket : ");

	// convert dns to IP
	server = gethostbyname(destAddr);
	if(server==NULL) return sitems;
	bzero((char *) &serverAddr.sin_addr.s_addr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(80);

	if(connect(s, (struct sockaddr*) &serverAddr, sizeof(serverAddr))<0) perror("Connect : ");
	if(send(s, request, strlen(request), 0)<strlen(request)) perror("Send : ");

	while(recv(s, buffer, 3000, 0)!=0){
		while(buffer!=NULL){
			// allocate memmory for new item
			if(counter==0) 
				sitems.items = (Item *) malloc(sizeof(Item)); 
			else
				sitems.items = (Item *) realloc(sitems.items, sizeof(Item)*(counter+1));
			// make sure data from other items is not read if data from current item is missing
			tmp = getBetweenTags(buffer, "<item>", "</item>");
			// parse ttle, link and description into item
			item.title = getBetweenTags(tmp, "<title>", "</title>");
			item.link  = getBetweenTags(tmp, "<link>", "</link>");
			item.description = getBetweenTags(tmp, "<description>", "</description>");
			// put item in array
			sitems.items[counter] = item; 
			// remove parsed item from buffer
			tmp = strstr(buffer, "</item>");
			strcpy(buffer, tmp); 
			// destroy tag so strstr wont find the same tag next round
			buffer[0] = '0';
			counter++;
		}
	}

	sitems.totalItems = counter;

	close(s);
	return sitems;
}

char *getBetweenTags(char *haystack, char *start, char *stop){
	char *tmp  = strstr(haystack, start);
	int i, c = strlen(tmp) - strlen(strstr(haystack, stop)) - strlen(start);

	char *newstr;
	newstr = malloc(sizeof(char)*c);

	for(i=0; i<c; i++)
		newstr[i] = tmp[i+strlen(start)];

	return newstr;
}

