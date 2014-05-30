/* scrape.c 
 * 2014-04-21
 * Robin Duda
 * Get tracker state.
 */

#ifndef _scrape_h
#define _scrape_h

 #include <errno.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include "urlparse.h" 
 #include "protocol_meta.h"
 #include "protocol_tcp.h"
 #include "protocol_udp.h"

//scrapes the trackerlist and stores the scrape data per tracker.
void tracker_scrape(swarm_t* swarm);


#endif