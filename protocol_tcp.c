/* protocol_tcp.c
 * 2014-04-21
 * Robin Duda
 *  UDP Tracker protocol.
 */

#include "protocol_tcp.h"


//construct a http scrape query
static int scrape_build(char request[256], char* info_hash, char* tracker) 
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

//construct a http announce query. [todo: add left, downloaded and event]
static void announce_build(char request[300], char* info_hash, char* peer_id, char* tracker, int listenport) 
{
    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    char* hash_escape = (char*) malloc(61);

    url_hostname(tracker, hostname);
    url_announce(tracker, announce);
    url_encode(info_hash, hash_escape);
                                                                        
    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&ip=127.0.0.1&downloaded=%d&left=%d&event=%s&numwant=50 HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, hash_escape, peer_id, listenport, 12008, 12379, "started", hostname);

    free(hash_escape);
    free(announce);
    free(hostname);
    return;
}

//read the scrape response.
static void scrape_response(int* sockfd, scrape_t* scrape)
{
    int num = 0;
    char recvbuf[2048];

    memset(recvbuf, '\0', sizeof(recvbuf));
    sleep(SCRAPE_TIME);

    if ((num = recv(*sockfd, recvbuf, sizeof(recvbuf)-1, MSG_DONTWAIT)) > 0)
    {
        recvbuf[num] = '\0';
        netstat_update(swarm->info_hash, INPUT, strlen(recvbuf));

        scrape->tracker->completed  = bdecode_value(recvbuf, "complete");
        scrape->tracker->downloaded = bdecode_value(recvbuf, "downloaded");
        scrape->tracker->incomplete = bdecode_value(recvbuf, "incomplete");

        //don't overestimate the peer count, most peers will be connected to every tracker in announce-list.
        if (scrape->swarm->completed < scrape->tracker->completed)
            scrape->swarm->completed = scrape->tracker->completed;
        if (scrape->swarm->incomplete < scrape->tracker->completed)
            scrape->swarm->incomplete = scrape->tracker->incomplete;
     }    
}

//read uncompressed peer entries.
static void announce_response(int* sockfd, announce_t* announce)
{
    int num, i, j;
    unsigned char data;
    unsigned short int port;
    char recvbuf[2048], seek[6], ip[4];

    memset(recvbuf, '\0', sizeof(recvbuf));
    sleep(ANNOUNCE_TIME);

    if ((num = recv(*sockfd, recvbuf, sizeof(recvbuf)-1, MSG_DONTWAIT)) > 0)
    {
        recvbuf[num] = '\0';

        netstat_update(announce->swarm->info_hash, INPUT, num);
        announce->tracker->interval   = bdecode_value(recvbuf, ":interval");
        announce->tracker->minterval  = bdecode_value(recvbuf, ":min interval");

        for (i = 0; i < num; i++)                           //seek
        {
            if (recvbuf[i] == ':')                          //match keyword
            {
                strncpy(seek, recvbuf+i, 6);
                seek[6] = '\0';

                if (strcmp(seek, ":peers") == 0)
                {   

                    i++;
                    while (recvbuf[i] != ':' && i < num)     //seek start of peerlist.
                        i++;
                    i++;

                    lock(&announce->swarm->peerlock);
                    while (i + 6 <= num && announce->swarm->peercount < MAX_SWARM_SIZE)
                    {                            //read IP, 4 ordered bytes.
                        for (j = 0; j < 4; j++)
                            ip[j] = recvbuf[i+j];

                        sprintf(announce->swarm->peer[announce->swarm->peercount].ip, "%hd.%hd.%hd.%hd", 
                              (unsigned char) ip[0], (unsigned char) ip[1], (unsigned char) ip[2], (unsigned char) ip[3]);

                        i += 4;
                        data = recvbuf[i];
                        port = 0;
                        port = (unsigned char) data << 8;
                        port += (unsigned char) recvbuf[i+1];
                        i += 2;

                        sprintf(announce->swarm->peer[announce->swarm->peercount].port, "%d", (unsigned) port);
                        announce->swarm->peercount++;
                    }
                    unlock(&announce->swarm->peerlock);
                    fflush(stdout);
                }
            }
        }       
    }
}

//scrape a tcp tracker
void tcp_scrape(scrape_t* scrape)
{
    int sockfd;
    struct addrinfo hints, *res;
    int port = 80, url_len = MAX_URL_LEN;
    char request[MAX_URL_LEN] = {0};
    char* portname = malloc(6);
    char* hostname = malloc(url_len); 
    scrape->tracker->alive = false;

    scrape_build(request, scrape->swarm->info_hash, scrape->tracker->url);
    url_hostname(scrape->tracker->url, hostname);
    url_port(scrape->tracker->url, &port);
    sprintf(portname, "%d", (unsigned int) port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(hostname, portname, &hints, &res) == 0)
    {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
        {
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
            {
                send(sockfd, request, strlen(request), 0);
                netstat_update(scrape->swarm->info_hash, OUTPUT, strlen(request));
                scrape_response(&sockfd, scrape);
            } 
            close(sockfd);
        }
    }
    free(hostname);
    free(portname);
}

//send a http announce query.
void tcp_announce(announce_t* announce)
{
    int   port = 80, sockfd, url_len = 200;
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    char request[300];
    struct addrinfo hints, *res;
    announce->tracker->alive = false;

    announce_build(request, announce->swarm->info_hash, announce->swarm->peer_id, announce->tracker->url, announce->swarm->listenport);
    url_hostname(announce->tracker->url, hostname);
    url_protocol(announce->tracker->url, protocol);
    url_port(announce->tracker->url, &port);
    sprintf(protocol, "%d", (unsigned int) port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
            
    //make the sock, perform the query, read the response.
    if (getaddrinfo(hostname, protocol, &hints, &res) == 0)
    {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
        {
            if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
            {
                send(sockfd, request, strlen(request), 0);
                netstat_update(announce->swarm->info_hash, OUTPUT, strlen(request));
                announce_response(&sockfd, announce);
            } 
            close(sockfd);
        }
    }

    announce->tracker->alive = true;        //ready for new announce, if not set the tracker is attempting to connect with kernel sock timeout. [20 min]
    free(hostname);
    free(protocol);
}
