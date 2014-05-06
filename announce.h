#ifndef _announce_h
#define _announce_h

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

//announce all trackers in trackerlist, algorithm for announcing only on trackers with
//enough peers not added yet.
int tracker_announce(swarm_t* swarm);

#endif