/* scrape.c 
 * 2014-04-21
 * Robin Duda
 * Get tracker state.
 */

 #include "scrape.h"


typedef struct 
{
    tracker_t* tracker;
    swarm_t* swarm;
    pthread_t thread;
} scrape_t;

//construct a http query
static int build(char request[200], char* info_hash, char* tracker) 
{
    char* path =     (char*) malloc(MAX_URL_LEN);
    char* hostname = (char*) malloc(MAX_URL_LEN);
    char* hash_escape = (char*) malloc(61);

    url_hostname(tracker, hostname);
    url_path(tracker, path);
    url_encode(info_hash, hash_escape);

                                //todo: add string replace, /announce from url_announce needs to be replaced by /scrape, this won't work on other than PHP servers.
    sprintf(request, "GET %s/scrape.php?info_hash=%s HTTP/1.1\r\nhost: %s\r\n\r\n", path, hash_escape, hostname);
    
    free(path);
    free(hostname);
    free(hash_escape);
    return strlen(request);
}

static void response(int* sockfd, scrape_t* scrape)
{
    int num = 0;
    char recvbuf[2048];

    memset(recvbuf, '\0', sizeof(recvbuf));
    sleep(SCRAPE_TIME);

    if ((num = recv(*sockfd, recvbuf, sizeof(recvbuf)-1, MSG_DONTWAIT)) > 0)
    {
        recvbuf[num] = '\0';
        netstat_update(INPUT, strlen(recvbuf), swarm->info_hash);

        scrape->tracker->completed  = bdecode_value(recvbuf, "complete");
        scrape->tracker->downloaded = bdecode_value(recvbuf, "downloaded");
        scrape->tracker->incomplete = bdecode_value(recvbuf, "incomplete");
        printf("\n[Scrape]\t%s\t[completed = %d, downloaded = %d, incomplete = %d]", 
            scrape->tracker->url, scrape->tracker->completed, 
            scrape->tracker->downloaded, scrape->tracker->incomplete);

        scrape->swarm->completed += scrape->tracker->completed;
        scrape->swarm->incomplete += scrape->tracker->incomplete;
     }    
}

static void* query(void* arg)
{
    scrape_t* scrape = (scrape_t*) arg;
    int port = 80, url_len = 200, sockfd;
    char request[200] = {0};
    char* hostname = malloc(url_len); 
    char* protocol = malloc(url_len);
    struct addrinfo hints, *res;
    scrape->tracker->alive = false;

    build(request, scrape->swarm->info_hash, scrape->tracker->url);
    url_hostname(scrape->tracker->url, hostname);
    url_protocol(scrape->tracker->url, protocol);
    url_port(scrape->tracker->url, &port);
    sprintf(protocol, "%d", (unsigned int) port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(hostname, protocol, &hints, &res) == 0)
    {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
        {
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
            {
                send(sockfd, request, strlen(request), 0);
                netstat_update(OUTPUT, strlen(request), scrape->swarm->info_hash);
                response(&sockfd, scrape);
            } 
            close(sockfd);
        }
    }

    scrape->tracker->alive = true;
    free(hostname);
    free(protocol);
    free(scrape);
    return arg;
}

//todo: scrape if interval has passed and is alive.
 void tracker_scrape(swarm_t* swarm)
 {
    scrape_t* scrape;
    int i = 1;

    strcpy(swarm->tracker[1].url, "http://127.0.0.1:80/tracker/announce.php");  //every other entry in trackers is empty? added local tracker temporary for testing.

    for (i = 0; i < MAX_TRACKERS; i++)
    {

        if (swarm->tracker[i].alive == true && strlen(swarm->tracker[i].url) > 5)
        {
            scrape = (scrape_t*) malloc(sizeof(scrape_t));
            scrape->tracker = &swarm->tracker[i];
            scrape->swarm = swarm;

            scrape->swarm->completed = 0;
            scrape->swarm->incomplete = 0;

            if(!(pthread_create(&scrape->thread, NULL, query, scrape)))
                printf("\nScraping: %s.", scrape->tracker->url);
            else
                swarm->tracker[i].alive = false;
        }
    }
    sleep(SCRAPE_TIME + 1);
 }