/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#include "announce.h"

//send a http query, the announce.
static void* announcer(void* arg)
{
    announce_t* announce = (announce_t*) arg;
    char* protocol = malloc(MAX_URL_LEN);

    url_protocol(announce->tracker->url, protocol);

    if (strcmp(protocol, "udp") == 0)
        udp_announce(announce);
    else
        if (strcmp(protocol, "http") == 0)
            tcp_announce(announce); 

    free(protocol);
    free(announce);
    return arg;
}


//launch an announce thread for every tracker. 
//[todo: track when announce-interval has expired.]
void tracker_announce(swarm_t* swarm) 
{
    announce_t* announce;
    int i;

    swarm_reset(swarm);  //clear stale peers

    for (i = 0; i < swarm->tinfo->_announce_list_count; i++)
    {
        if (swarm->tracker[i].alive == true)
        {
            announce = (announce_t*) malloc(sizeof(announce_t));
            announce->tracker = &swarm->tracker[i];
            announce->swarm = swarm;

            if(pthread_create(&announce->thread, NULL, announcer, announce))
                swarm->tracker[i].alive = false;
        }
    }
    sleep(ANNOUNCE_TIME + 1);
} 
