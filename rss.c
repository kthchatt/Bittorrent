#include "rss.h"

Items getFeed(char *destAddr, char *dir){
	Item item;
	Items sitems;
	struct hostent *server;
	struct sockaddr_in serverAddr;
	int s, i, counter=0, slen=sizeof(server);
	char *request, *buffer, *tmp, tmpBuffer[2048];

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

	// load entire feed into buffer (max 100*2048 bytes)
	buffer = malloc(sizeof(tmpBuffer)+1);
	while(recv(s, tmpBuffer, 2048, 0)!=0 && counter < 500){
		buffer = realloc(buffer, sizeof(tmpBuffer)*(counter+1)+1);
		strcpy(buffer, tmpBuffer);
		counter++;
	}
	counter = 0;

	// parse buffer into items (max 50 items are loaded)
	while(strstr(buffer, "<item>")!=NULL && counter < 50){
		// allocate memmory for new item
		if(counter==0) 
			sitems.items = (Item *) malloc(sizeof(Item)); 
		else
			sitems.items = (Item *) realloc(sitems.items, sizeof(Item)*(counter+1));
		// make sure data from other items is not read if data from current item is missing
		tmp = malloc(strlen(getBetweenTags(buffer, "<item>", "</item>"))+1);
		tmp = getBetweenTags(buffer, "<item>", "</item>");
		// parse title, link and description into item
		item.title = getBetweenTags(tmp, "<title>", "</title>");
		item.link  = getBetweenTags(tmp, "<link>", "</link>");
		item.description = getBetweenTags(tmp, "<description>", "</description>");
		// put item in array
		sitems.items[counter] = item; 
		// remove parsed item from buffer
		free(tmp);
		tmp = strstr(buffer, "</item>");
		strcpy(buffer, tmp); 
		// destroy tag so strstr wont find the same tag next round
		buffer[0] = '0';
		counter++;
	}
	//sitems.totalItems = counter;

	free(buffer);
	close(s);
	return sitems;
}

char *getBetweenTags(char *haystack, char *start, char *stop){
	char *tmp  = strstr(haystack, start);
	int i, c = strlen(tmp) - strlen(strstr(haystack, stop)) - strlen(start);
	char *newstr;
	newstr = malloc(c+1);

	for(i=0; i<c; i++)
		newstr[i] = tmp[i+strlen(start)];

	return newstr;
}

int main(){
	int i;
	Items test = getFeed("showrss.info", "/feeds/27.rss");
	
	//for(i=0; i<test.totalItems; i++)
	//	puts(test.items[i].title);		// print all titles found in feed
	
	return 1;
}

