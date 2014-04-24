/* scrape.c 
 * 2014-04-21
 * Robin Duda
 * Get tracker state.
 */


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
 #include "scrape.h"
 #include "swarm.h"

//construct a http query
static int build(char request[200], char info_hash[20], char tracker[MAX_URL_LEN]) 
{
    char* path =     (char*) malloc(MAX_URL_LEN);
    char* hostname = (char*) malloc(MAX_URL_LEN);

    url_hostname(tracker, hostname);
    url_path(tracker, path);

    sprintf(request, "GET %s/scrape.php?info_hash=%s HTTP/1.1\r\nhost: %s\r\n\r\n", path, info_hash, hostname);


    free(path);
    free(hostname);
    return strlen(request);
}

static void response(int* sockfd, swarm_t* swarm)
{
    int num;
    char recvbuf[2048];

    memset(recvbuf, '0', sizeof(recvbuf));

    if ((num = read(*sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
    {
        recvbuf[num] = 0;
        printf("%s", recvbuf);
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
        if (strlen(swarm->tracker[i]) > 0)
        {
            build(request, swarm->info_hash, swarm->tracker[i]);
            url_hostname(swarm->tracker[i], hostname);
            url_protocol(swarm->tracker[i], protocol);
            url_port(swarm->tracker[i], &port);

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
                    response(&sockfd, swarm);
                } 
            }
        }
    }

    free(hostname);
    free(protocol);
}

 void tracker_scrape(swarm_t* swarm)
 {
    query(swarm);
 }