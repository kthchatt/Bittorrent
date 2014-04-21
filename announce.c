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


//ip is set to 0 for non-proxy connections.
//TODO: Return list of peers.
int tracker_announce(char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 
{
    int sockfd = 0, n = 0;
    char recvbuf[1024];
    struct addrinfo hints, *res;

    int url_len = strlen(tracker);
    char* announce = (char*) malloc(url_len);
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    int   port = 80;

    url_announce(tracker, announce);
    url_hostname(tracker, hostname);
    url_protocol(tracker, protocol);
    url_port(tracker, &port);

    printf("\n\nannounce request!\nurl: %s\nhostname: %s\nannounce: %s\nprotocol: %s\nport: %d\n\n", tracker, hostname, announce, protocol, port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(hostname, protocol, &hints, &res);

    printf("GETADDRINFO_OK");
    fflush(stdout);

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    //TODO: bind to port, include port in announce request.

    printf("SOCKFD_OK");
    fflush(stdout);

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    printf("RequestBuidler");
    fflush(stdout);

    //add IP ?

    char request[200];
    strcat(request, "GET ");
    strcat(request, announce);
    strcat(request, "?info_hash=");
    strcat(request, info_hash);
    strcat(request, "&peer_id=");
    strcat(request, peer_id);
    strcat(request, "&port=");
    strcat(request, "8080");
    strcat(request, "&downloaded=");
    strcat(request, "0");
    strcat(request, "&left=");
    strcat(request, "12852");
    strcat(request, "&event=");
    strcat(request, event);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "host: ");
    strcat(request, hostname);
    strcat(request, "\r\n\r\n");

    int len, bytes_sent, i = 0;
    len = strlen(request);
    bytes_sent = send(sockfd, request, len, 0);

    memset(recvbuf, '0',sizeof(recvbuf));
    while ((n = read(sockfd, recvbuf, 2)) > 0) //sizeof(recvbuf)-1) = nbytes (1)
    {
        recvbuf[n] = 0;
        if(fputs(recvbuf, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
            return 1;
        }

        printf("[%d]", recvbuf);
        i++;
    } 

    if (n < 0)
    {
        printf("\n Read error \n");
        return 1;
    }  

    return 0;
}

/*
//main for testing.
int main(int argc, char *argv[])
{
    if (announce("protocol://retracker.hq.ertelecom.ru:port/announce-url","iiiinnnnffffoooohashkkkkk", "peerIDaaaabbbbcdeeff", "0", "Started", 0, 120582) != 0)
    {
        printf("Announce Error.");
    }
    
    return 0;
}*/