/* scrape.c 
 * 2014-04-21
 * Robin Duda
 * Get tracker state.
 */

 #include "scrape.h"

//construct a http query
static int build(char request[200], char info_hash[21], char tracker[MAX_URL_LEN]) 
{
    char* path =     (char*) malloc(MAX_URL_LEN);
    char* hostname = (char*) malloc(MAX_URL_LEN);
    char* hash_escape = (char*) malloc(61);

    url_hostname(tracker, hostname);
    url_path(tracker, path);
    url_encode(info_hash, hash_escape);

    sprintf(request, "GET %s/scrape.php?info_hash=%s HTTP/1.1\r\nhost: %s\r\n\r\n", path, hash_escape, hostname);

    free(path);
    free(hostname);
    free(hash_escape);
    return strlen(request);
}

static void response(int* sockfd, swarm_t* swarm, int index)
{
    int num = 0;
    char recvbuf[2048];

    memset(recvbuf, '\0', sizeof(recvbuf));

    if ((num = read(*sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
    {
        recvbuf[num] = '\0';
        netstat_update(INPUT, strlen(recvbuf), swarm->info_hash);

        swarm->tracker[index].scrape_completed  = bdecode_value(recvbuf, "complete");
        swarm->tracker[index].scrape_downloaded = bdecode_value(recvbuf, "downloaded");
        swarm->tracker[index].scrape_incomplete = bdecode_value(recvbuf, "incomplete");
        printf("\n[Scrape]\t%s\t[completed = %d, downloaded = %d, incomplete = %d]", 
            swarm->tracker[index].url, swarm->tracker[index].scrape_completed, 
            swarm->tracker[index].scrape_downloaded, swarm->tracker[index].scrape_incomplete);
     }    
}

static void query(swarm_t* swarm)
{
	int   port = 80, url_len = 200, i, sockfd;
    char request[200] = {0};
    char* hostname = (char*) malloc(url_len); 
    char* protocol = (char*) malloc(url_len);
    struct addrinfo hints, *res;

    //loop through tracker urls and scrape them all.
    for (i = 0; i < MAX_TRACKERS; i++)
    {
        if (strlen(swarm->tracker[i].url) > 0)
        {
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

            if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
            {
                if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
                {
                    send(sockfd, request, strlen(request), 0);
                    netstat_update(OUTPUT, strlen(request), swarm->info_hash);
                    response(&sockfd, swarm, i);
                } 
            }
        }
    }
    free(hostname);
    free(protocol);
    close(sockfd);
}

 void tracker_scrape(swarm_t* swarm)
 {
    query(swarm);
 }