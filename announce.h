/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#ifndef _announce_h
#define _announce_h

 #include <pthread.h>
 #include "urlparse.h" 
 #include "swarm.h"
 #include "protocol_tcp.h"
 #include "protocol_udp.h"
 #include "protocol_meta.h"

//announce on trackers associated with the swarm, using the swarm's infohash.
void tracker_announce(swarm_t* swarm);

#endif