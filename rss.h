#ifndef _rss_h
#define _rss_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
 
typedef struct Items{
	int totalItems;
	Item *items;
}Items;

typedef struct Item{
	char *title,
		 *description,
		 *link;
}Item;

// Returns array of all rss items found and the total amount of items
Items getFeed(char *destAddr, char *dir);
// returns string between two tags
char *getBetweenTags(char *haystack, char *start, char *stop);

// EXAMPLE //
/*
int main(){
	int i;
	Items test = getFeed("showrss.info", "/feeds/27.rss");
	
	for(i=0; i<test.totalItems; i++)
		puts(test.items[i].title);		// print all titles found in feed
	
	return 1;
}
*/

#endif