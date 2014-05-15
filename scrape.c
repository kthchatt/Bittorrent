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
int build(char request[200], char* info_hash, char* tracker) 
{
    printf("\nLeveloped. [%s]", tracker); fflush(stdout);
    char* path =     (char*) malloc(MAX_URL_LEN);
    char* hostname = (char*) malloc(MAX_URL_LEN);
    char* hash_escape = (char*) malloc(61);


    sleep(5);
    printf("\nLeveloped.2"); fflush(stdout);
    url_hostname(tracker, hostname);
    url_path(tracker, path);
    url_encode(info_hash, hash_escape);
    printf("\nLeveloped.3"); fflush(stdout);
    sprintf(request, "GET %s/scrape.php?info_hash=%s HTTP/1.1\r\nhost: %s\r\n\r\n", path, hash_escape, hostname);
    printf("\nLeveloped.4"); fflush(stdout);
    free(path);
    free(hostname);
    free(hash_escape);
    printf("\nLeveloped.5"); fflush(stdout);
    return strlen(request);
}

void response(int* sockfd, swarm_t* swarm, tracker_t* tracker)
{
    int num = 0;
    char recvbuf[2048];

    memset(recvbuf, '\0', sizeof(recvbuf));

    printf("\nWaiting for response."); fflush(stdout);
    sleep(SCRAPE_TIME);
    printf("\nTimed out, now for reading!"); fflush(stdout);

    if ((num = recv(*sockfd, recvbuf, sizeof(recvbuf)-1, MSG_DONTWAIT)) > 0)
    {
        printf("\nThere was some bytes to be received."); fflush(stdout);
        recvbuf[num] = '\0';
        netstat_update(INPUT, strlen(recvbuf), swarm->info_hash);

        tracker->scrape_completed  = bdecode_value(recvbuf, "complete");
        tracker->scrape_downloaded = bdecode_value(recvbuf, "downloaded");
        tracker->scrape_incomplete = bdecode_value(recvbuf, "incomplete");
        printf("\n[Scrape]\t%s\t[completed = %d, downloaded = %d, incomplete = %d]", 
            tracker->url, tracker->scrape_completed, 
            tracker->scrape_downloaded, tracker->scrape_incomplete);
     }    
     else 
        printf("\nThere was none bytes received."); fflush(stdout);

    printf("\nThe response was completed."); fflush(stdout);
}

/*static void query(swarm_t* swarm)
{
	int   port = 80, url_len = 200, i, sockfd;
    char request[200] = {0};
    char* hostname = (char*) malloc(url_len); 
    char* protocol = (char*) malloc(url_len);
    struct addrinfo hints, *res;

    //loop through tracker urls and scrape them all.
    for (i = 0; i < MAX_TRACKERS; i++)
    {
        printf("\nScraping: %s", swarm->tracker[i].url); fflush(stdout);
        if (strlen(swarm->tracker[i].url) > 0)
        {
            printf("\nThe strlen was more than 0 for url."); fflush(stdout);
            build(request, swarm->info_hash, swarm->tracker[i].url);
            url_hostname(swarm->tracker[i].url, hostname);
            url_protocol(swarm->tracker[i].url, protocol);
            url_port(swarm->tracker[i].url, &port);
            sprintf(protocol, "%d", (unsigned int) port);

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            getaddrinfo(hostname, protocol, &hints, &res);

            printf("\nSockening it up."); fflush(stdout);

            if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
            {
                printf("\nConnening it up.. "); fflush(stdout);
                if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
                {
                    printf("\nConning and now time to send."); fflush(stdout);
                    send(sockfd, request, strlen(request), 0);
                    printf("\nNetstateing it up.. "); fflush(stdout);
                    netstat_update(OUTPUT, strlen(request), swarm->info_hash);
                    printf("\nPreparing for the response."); fflush(stdout);
                    response(&sockfd, swarm, i);
                } 
                else
                    printf("\nDude, the conn failed! "); fflush(stdout);
                close(sockfd);
            }
        }
    }
    free(hostname);
    free(protocol);
}*/

void* query(void* arg)
{
    scrape_t* scrape = (scrape_t*) arg;
    int port = 80, url_len = 200, sockfd;
    char request[200] = {0};
    char* hostname = malloc(url_len); 
    char* protocol = malloc(url_len);
    struct addrinfo hints, *res;
    swarm->tracker->alive = false;

            printf("pre-scrappen!\n"); fflush(stdout);
            printf("\nScrappening!: %s", scrape->tracker->url); fflush(stdout);
            build(request, scrape->swarm->info_hash, scrape->tracker->url);
            printf("\nBuild completed.!");
            url_hostname(scrape->tracker->url, hostname);
            url_protocol(scrape->tracker->url, protocol);
            url_port(scrape->tracker->url, &port);
            sprintf(protocol, "%d", (unsigned int) port);

            printf("\nScrappen2"); fflush(stdout);

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;
            getaddrinfo(hostname, protocol, &hints, &res);

            printf("\nSockening it up."); fflush(stdout);

            if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
            {
                printf("\nConnening it up.. "); fflush(stdout);
                if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
                {
                    printf("\nConning and now time to send."); fflush(stdout);
                    send(sockfd, request, strlen(request), 0);
                    printf("\nNetstateing it up.. "); fflush(stdout);
                    netstat_update(OUTPUT, strlen(request), scrape->swarm->info_hash);
                    printf("\nPreparing for the response."); fflush(stdout);
                    response(&sockfd, scrape->swarm, scrape->swarm->tracker);
                } 
                else
                    printf("\nDude, the conn failed! "); fflush(stdout);
                close(sockfd);
            }


    scrape->tracker->alive = true;
    free(hostname);
    free(protocol);
    free(scrape);
}

 void tracker_scrape(swarm_t* swarm)
 {
    scrape_t* scrape;
    int i = 1;

    //todo: check the tracker urls, there may be some bad trackers.

    for (i = 0; i < MAX_TRACKERS; i++)
    {
        if (swarm->tracker[i].alive == true)
        {
            scrape = (scrape_t*) malloc(sizeof(scrape_t));
            scrape->tracker = &swarm->tracker[i];
            scrape->swarm = swarm;

            if (strlen(swarm->info_hash) > 0)
            {
                if(!(pthread_create(&scrape->thread, NULL, query, scrape)))
                    printf("\nScraping: %s.", scrape->tracker->url);
                else
                    swarm->tracker[i].alive = false;
            }
        }
    }
    sleep(SCRAPE_TIME + 1);
    printf("\nScrape is done.??"); fflush(stdout);
 }