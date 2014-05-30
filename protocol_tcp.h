/* protocol_tcp.c
 * 2014-04-21
 * Robin Duda
 *  TCP Tracker protocol.
 */

#ifndef _protocol_tcp_
#define _protocol_tcp_

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

void tcp_announce(announce_t* announce); //announce to a tcp tracker.
void tcp_scrape(scrape_t* scrape);		 //scrape tcp tracker.
		
#endif