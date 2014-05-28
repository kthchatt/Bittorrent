/* rss2.c 
 * RSS Reader.
 * Author: Anton L.
 * 
 * Refactor: Robin D.
 * 	Fixed bugs; 
 *		Not fetching whole RSS document, 
 *  	Cruft at titles, 
 *		Segmentation fault in extract_content, (was not error checking strstr)
 *  	Fixed incorrect memory allocations.
 *
 * 	Additions/Modifications
 *  	RSS Fething now threaded.
 *  	Content extractor no longer public.
 *  	Variable RSS sources.
 *  	Source formatted as Allman, changed variable/method names.
 *  	ReAdded item count.
 *  	Rewrote socket code for clarity and error tolerance.
 */

#ifndef _rss2_h
#define _rss2_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "protocol_meta.h"

#define RSS_TIMEOUT 2
#define MAX_ITEMS 64
#define BYTEBUFFER 65535
#define RSS_PORT "80"

//structs renamed to avoid items.items hierarchy.
typedef struct 
{
	char *title,
		 *description,
		 *link;
} item_t;

typedef struct 
{
	int item_count;
	item_t* item;	
	char* host;
	char* uri;	
} rss_t;

/* queried for deletion. Reason: outdated.  ~RD
// Returns array of all rss items found and the total amount of items
Items getFeed(char *destAddr, char *dir); */

//public function, is threaded. call with: items_t x; rss_feed(&x); x.item[0].attribute;  ~RD
void rss_fetch(rss_t* test);

/* queried for deletion. Reason: should not be shared. ~RD
// returns string between two tags
char *getBetweenTags(char *haystack, char *start, char *stop);
*/

#endif