/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#include "announce.h"

 typedef struct 
{
    tracker_t* tracker;
    swarm_t* swarm;
    pthread_t thread;
} announce_t;


//construct a http query
static void build(char request[300], char* info_hash, char* peer_id, char* tracker, int listenport) 
{
    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    char* hash_escape = (char*) malloc(61);

    url_hostname(tracker, hostname);
    url_announce(tracker, announce);
    url_encode(info_hash, hash_escape);

    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&ip=127.0.0.1&downloaded=%d&left=%d&event=%s&numwant=200 HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, hash_escape, peer_id, listenport, 12008, 12379, "started", hostname);


    free(hash_escape);
    free(announce);
    free(hostname);
    return;
}


//todo: read peer data into swarm, save intervals.
//read uncompressed 
static void response(int* sockfd, announce_t* announce)
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

        netstat_update(INPUT, num, announce->swarm->info_hash);
        announce->tracker->interval   = bdecode_value(recvbuf, ":interval");
        announce->tracker->minterval  = bdecode_value(recvbuf, ":min interval");

        printf("\n[Announce]\t%s \t[Interval = %d, Min Interval = %d]", 
                announce->tracker->url, 
                announce->tracker->interval, 
                announce->tracker->minterval);

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

                    lock(&announce->swarm->peerlock);
                    while (i + 6 <= num)           //keep reading
                    {
                        //read IP, 4 ordered bytes.
                        for (j = 0; j < 4; j++)
                            ip[j] = recvbuf[i+j];

                        sprintf(announce->swarm->peer[announce->swarm->peercount].ip, "%hd.%hd.%hd.%hd", 
                              (unsigned char) ip[0], (unsigned char) ip[1], (unsigned char) ip[2], (unsigned char) ip[3]);
                        
                        printf("\nip = [%s]\n", announce->swarm->peer[announce->swarm->peercount].ip);

                        i += 4;
                        data = recvbuf[i];
                        port = 0;
                        port = (unsigned char) data << 8;
                        port += (unsigned char) recvbuf[i+1];
                        i += 2;

                        sprintf(announce->swarm->peer[announce->swarm->peercount].port, "%d", (unsigned) port);
                        printf("port [%s]", announce->swarm->peer[announce->swarm->peercount].port);
                        announce->swarm->peercount++;
                    }
                    unlock(&announce->swarm->peerlock);
                    printf("\n");
                    fflush(stdout);
                }
            }
        }       
    }
}

//send a http query
static void* query(void* arg)
{
    announce_t* announce = (announce_t*) arg;
    int   port = 80, sockfd, url_len = 200;
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    char request[300];
    struct addrinfo hints, *res;
    announce->tracker->alive = false;

    build(request, announce->swarm->info_hash, announce->swarm->peer_id, announce->tracker->url, announce->swarm->listenport);
    url_hostname(announce->tracker->url, hostname);
    url_protocol(announce->tracker->url, protocol);
    url_port(announce->tracker->url, &port);
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
                netstat_update(OUTPUT, strlen(request), announce->swarm->info_hash);
                response(&sockfd, announce);
            } 
            close(sockfd);
        }
    }

    announce->tracker->alive = true;
    free(hostname);
    free(protocol);
    free(announce);
    return arg;
}

//todo: scrape if interval has passed and is alive.
void tracker_announce(swarm_t* swarm) 
{
    announce_t* announce;
    int i;

    swarm_reset(swarm);  //clear all current peers, WIP

    strcpy(swarm->tracker[1].url, "http://127.0.0.1:80/tracker/announce.php");

    for (i = 0; i < MAX_TRACKERS; i++)
    {
        if (swarm->tracker[i].alive == true && strlen(swarm->tracker[i].url) > 5)
        {
            announce = (announce_t*) malloc(sizeof(announce_t));
            announce->tracker = &swarm->tracker[i];
            announce->swarm = swarm;

            if(!(pthread_create(&announce->thread, NULL, query, announce)))
                printf("\nScraping: %s.", announce->tracker->url);
            else
                swarm->tracker[i].alive = false;
        }
    }
    sleep(ANNOUNCE_TIME + 1);
} 
