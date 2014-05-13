/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#include "announce.h"

 //todo: save a peerlist with ip:port and info_hash.

//construct a http query
static void build(char request[300], char info_hash[21], char peer_id[21], char tracker[MAX_URL_LEN]) 
{
    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    char* hash_escape = (char*) malloc(61);

    url_hostname(tracker, hostname);
    url_announce(tracker, announce);
    url_encode(info_hash, hash_escape);

    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&ip=129.0.0.1&downloaded=%d&left=%d&event=%s&numwant=200 HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, hash_escape, peer_id, swarm->listenport, 12008, 12379, "started", hostname);


    free(hash_escape);
    free(announce);
    free(hostname);
    return;
}


//todo: read peer data into swarm, save intervals.
//read uncompressed 
static void response(int* sockfd, swarm_t* swarm, int index)
{
    int num, i, j;
    unsigned char data;
    unsigned short int port;
    char recvbuf[2048], seek[6], ip[4];

    memset(recvbuf, '\0', sizeof(recvbuf));

    if ((num = read(*sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
    {
        recvbuf[num] = '\0';

        netstat_update(INPUT, num, swarm->info_hash);
        swarm->tracker[index].announce_interval   = bdecode_value(recvbuf, ":interval");
        swarm->tracker[index].announce_minterval  = bdecode_value(recvbuf, ":min interval");

        printf("\n[Announce]\t%s \t[Interval = %d, Min Interval = %d]", 
                swarm->tracker[index].url, 
                swarm->tracker[index].announce_interval, 
                swarm->tracker[index].announce_minterval);

        for (i = 0; i < num; i++)   //seek
        {
            if (recvbuf[i] == ':') //match keyword
            {
                strncpy(seek, recvbuf+i, 6);
                seek[6] = '\0';

                if (strcmp(seek, ":peers") == 0)
                {   

                    i++;
                    while (recvbuf[i] != ':')
                        i++;
                    i++;

                    while (i + 6 <= num)           //keep reading
                    {
                        //read IP, 4 ordered bytes.
                        for (j = 0; j < 4; j++)
                            ip[j] = recvbuf[i+j];

                        sprintf(swarm->peer[swarm->peercount].ip, "%hd.%hd.%hd.%hd", 
                              (unsigned char) ip[0], (unsigned char) ip[1], (unsigned char) ip[2], (unsigned char) ip[3]);
                        
                        printf("\nip = [%s]\n", swarm->peer[swarm->peercount].ip);

                        i += 4;
                        data = recvbuf[i];
                        port = 0;
                        port = (unsigned char) data << 8;
                        port += (unsigned char) recvbuf[i+1];
                        i += 2;

                        sprintf(swarm->peer[swarm->peercount].port, "%d", (unsigned) port);
                        printf("port [%s]", swarm->peer[swarm->peercount].port);
                        swarm->peercount++;
                    }
                    printf("\n");
                    fflush(stdout);
                }
            }
        }       
    }
}

//send a http query
static void query(swarm_t* swarm)
{
    int   port = 80, sockfd, url_len = 200, i;
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    char request[300];
    struct addrinfo hints, *res;


    for (i = 0; i < MAX_TRACKERS; i++)
    {
        if (strlen(swarm->tracker[i].url) > 0)
        {
            build(request, swarm->info_hash, swarm->peer_id, swarm->tracker[i].url);
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

int tracker_announce(swarm_t* swarm) 
{
    //todo: swarm_reset? query should check if the peer already exists. (threads will die!)
    //in swarm_reset check maxpeers before adding a new peer, if full call clear_stale_peers. 
    //call clear_stale_peers before announcing.

    swarm_reset(swarm);  //clear all current peers
    query(swarm);   //bound port
    return 0;
} 
