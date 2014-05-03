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
#include <pthread.h>

#define TIMEOUT 1500000
#define MAX_ITEMS 50
#define BYTEBUFFER 65535
#define PORT "80"


//todo: depending on access method a mutex might be required for items_t !

//structs renamed to avoid items.items hierarchy.
typedef struct {
	char *title,
		 *description,
		 *link;
} item_t;

typedef struct {
	int item_count;
	item_t* item;		
} items_t;

//global variables allows the changing of source. ~RD
char* host = "showrss.info";
char* uri = "/feeds/27.rss";

/* queried for deletion. Reason: outdated.  ~RD
// Returns array of all rss items found and the total amount of items
Items getFeed(char *destAddr, char *dir); */

//public function, is threaded. call with: items_t xxx; rss_feed(&xxx); xxx.item[0].attribute;  ~RD
void rss_feed(items_t* test);

/* queried for deletion. Reason: should not be public. ~RD
// returns string between two tags
char *getBetweenTags(char *haystack, char *start, char *stop);
*/

#endif