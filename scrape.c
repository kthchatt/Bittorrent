/* scrape.c 
 * 2014-04-21
 * Robin Duda
 * Get tracker state.
 */

 #include "scrape.h"


// determine protocol, launch udp_scrape or tcp_scrape
static void* query(void* arg)
{
    scrape_t* scrape = (scrape_t*) arg;
    char* protocol = malloc(MAX_URL_LEN);
    
    url_protocol(scrape->tracker->url, protocol);

    if (strcmp(protocol, "udp") == 0)
        udp_scrape(scrape);
    else
        if (strcmp(protocol, "http") == 0)
        tcp_scrape(scrape);

    scrape->tracker->alive = true;
    free(protocol);
    free(scrape);
    return arg;
}

//[todo: use the interval stored in tracker for scraping individual peers.]
 void tracker_scrape(swarm_t* swarm)
 {
    scrape_t* scrape;
    int i = 1;

    swarm->completed = 0;
    swarm->incomplete = 0; 

    for (i = 0; i < swarm->tinfo->_announce_list_count; i++)
    {
        if (swarm->tracker[i].alive == true)
        {
            scrape = (scrape_t*) malloc(sizeof(scrape_t));
            scrape->tracker = &swarm->tracker[i];
            printf("\nScraping: %s", scrape->tracker->url);
            scrape->swarm = swarm;
            scrape->tracker->alive = false;

            if (pthread_create(&scrape->thread, NULL, query, scrape))
                printf("\nScraping: %s Failed.", scrape->tracker->url);
            else
                swarm->tracker[i].alive = false;
        }
    }
    sleep(SCRAPE_TIME + 1);
 }