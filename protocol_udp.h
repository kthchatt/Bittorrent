/* protocol_udp.c
 * 2014-05-30
 * Robin Duda
 *  UDP Tracker protocol.
 */
 
#ifndef _protocol_udp_
#define _protocol_udp_

 #include <pthread.h>
 #include <netdb.h>
 #include <unistd.h>
 #include <errno.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <arpa/inet.h> 
 #include "urlparse.h" 
 #include "protocol_meta.h"
 #include "netstat.h"

void udp_announce(announce_t* announce);  	//announce to tcp tracker.
void udp_scrape(scrape_t* scrape);			//scrape tcp tracker.

#endif