/* announce.c 
 * 2014-04-16
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Announces presence to tracker and fetches peers in the swarm.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include "urlparse.h"
#include "announce.h"

 //todo: save a peerlist with ip:port and info_hash.

//construct a http query
static int build(char request[200], char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 
{
    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    int port = rand()%64519+1024;    //bound port: listener for info_hash.

    url_hostname(tracker, hostname);
    url_announce(tracker, announce);

    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&downloaded=%d&left=%d&event=%s HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, info_hash, peer_id, port, downloaded, left, event, hostname);

    //printf("\n\n%s\n%s\n%s\n%s\n\n", request, tracker, hostname, announce);


    free(announce);
    free(hostname);

    return strlen(request);
}

//send a http query
static void query(char request[200], char* tracker, int* sockfd)
{
    int n = 0, port = 80, url_len = strlen(tracker);
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    struct addrinfo hints, *res;

    url_hostname(tracker, hostname);
    url_protocol(tracker, protocol);
    url_port(tracker, &port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(hostname, protocol, &hints, &res);

    if ((*sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
    {
        if (connect(*sockfd, res->ai_addr, res->ai_addrlen) > -1)
        {
            send(*sockfd, request, strlen(request), 0);
        } 
    }

    free(hostname);
    free(protocol);
}

//read uncompressed 
static void response(int* sockfd)
{
    int num, port, i, j, data;
    char recvbuf[2048], seek[5];
    char* seekpos;

    memset(recvbuf, '0', sizeof(recvbuf));

    if ((num = read(*sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
    {
        recvbuf[num] = 0;
        printf("%s", recvbuf);

        for (i = 0; i < num; i++)   //seek
        {
            if (recvbuf[i] == ':') //match keyword
            {
                seekpos = &recvbuf[i];
                strncpy(seek, recvbuf+i, 5);

                if (strcmp(seek, ":peer") == 0)
                {   
                    i++;
                    while (recvbuf[i] != ':')
                        i++;
                    i++;


                    while (i + 6 < num)           //keep reading
                    {
                        port = 0;

                        printf("\nIP: ");

                        //read IP, 4 ordered bytes.
                        for (j = 0; j < 4; j++)
                        {
                            data = recvbuf[i+j];
                            printf("%d.", data);    //todo: STORE IP
                        }

                        i += 4;
                        data = recvbuf[i];
                        port = (data * 256) ;
                        port += recvbuf[i+1];
                        i += 2;

                        printf("port: %d", port);   //todo: STORE PORT
                    }
                    printf("\n");
                }
            }
        }       
    }
}

//ip is set to 0 for non-proxy connections.
//TODO: Return list of peers.
int tracker_announce(char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 
{
    int sockfd = 0; 
    char request[200];

    build(request, tracker, info_hash, peer_id, ip, event, downloaded, left);   //bound port
    query(request, tracker, &sockfd);                                                           //target port in url
    response(&sockfd);                                                                                 //todo: bind/listen
    return 0;
} 
