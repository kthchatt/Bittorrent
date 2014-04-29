/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#include "announce.h"

 //todo: save a peerlist with ip:port and info_hash.

//construct a http query
static int build(char request[200], char info_hash[20], char peer_id[20], char tracker[MAX_URL_LEN]) 
{
    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    int port = rand()%64519+1024;    //bound port: listener for info_hash.

    url_hostname(tracker, hostname);
    url_announce(tracker, announce);

    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&downloaded=%d&left=%d&event=%s&numwant=10 HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, info_hash, peer_id, port, 0, 12379, "started", hostname);

    free(announce);
    free(hostname);

    return strlen(request);
}


//todo: read peer data into swarm, save intervals.
//read uncompressed 
static void response(int* sockfd, swarm_t* swarm, int index)
{
    int num, i, j;
    unsigned char data;
    unsigned short int port;
    char recvbuf[2048], seek[5], ip[4];
    char* seekpos;

    memset(recvbuf, '\0', sizeof(recvbuf));

    if ((num = read(*sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
    {
        recvbuf[num] = '\0';

        swarm->tracker[index].announce_interval   = bdecode_value(recvbuf, ":interval");
        swarm->tracker[index].announce_minterval  = bdecode_value(recvbuf, ":min interval");

        printf("\n%s \t[Interval = %d, Min Interval = %d]", 
                swarm->tracker[index].url, 
                swarm->tracker[index].announce_interval, 
                swarm->tracker[index].announce_minterval);

        for (i = 0; i < num; i++)   //seek
        {
            if (recvbuf[i] == ':') //match keyword
            {
                seekpos = &recvbuf[i];
                strncpy(seek, recvbuf+i, 6);

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

                        sprintf(swarm->ip[swarm->peercount], "%d.%d.%d.%d", 
                              (unsigned) ip[0], (unsigned) ip[1], (unsigned) ip[2], (unsigned) ip[3]);

                        printf("\nip = [%s]\n", swarm->ip[swarm->peercount]);

                        i += 4;
                        data = recvbuf[i];
                        port = 0;
                        port = (unsigned char) data << 8;
                        port += (unsigned char) recvbuf[i+1];
                        i += 2;

                        sprintf(swarm->port[swarm->peercount], "%d", (unsigned) port);
                        printf("port [%s]", swarm->port[swarm->peercount]);
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
    int n = 0, port = 80, sockfd, url_len = 200, i;
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    char request[200];
    struct addrinfo hints, *res;


    for (i = 0; i < MAX_TRACKERS; i++)
    {
        if (strlen(swarm->tracker[i].url) > 0)
        {
            build(request, swarm->info_hash, swarm->peer_id, swarm->tracker[i].url);
            url_hostname(swarm->tracker[i].url, hostname);
            url_protocol(swarm->tracker[i].url, protocol);
            url_port(swarm->tracker[i].url, &port);

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
                    response(&sockfd, swarm, i);
                } 
            }
        }
    }
    free(hostname);
    free(protocol);
    close(sockfd);
}

//ip is set to 0 for non-proxy connections.
//TODO: Return list of peers.
int tracker_announce(swarm_t* swarm) 
{
    swarm_reset(swarm);  //clear all current peers
    query(swarm);   //bound port                                                                           //todo: bind/listen
    return 0;
} 
