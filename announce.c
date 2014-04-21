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

 //todo: save a peerlist with ip:port and info_hash.

void debug(int postal) 
{ 
    printf("\n__stack_%d_safe__\n", postal); 
    fflush(stdout); 
}


int build(char request[200], char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 
{
    debug(0);


    char* announce = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    int port = 80;

    url_port(tracker, &port);
    url_hostname(tracker, hostname);
    url_announce(tracker, announce);

    sprintf(request, "GET %s?info_hash=%s&peer_id=%s&port=%d&downloaded=%d&left=%d&event=%s HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    announce, info_hash, peer_id, port, downloaded, left, event, hostname);
    
    /*strcat(request, "GET ");
    strcat(request, announce);
    strcat(request, "?info_hash=");
    strcat(request, info_hash);
    strcat(request, "&peer_id=");
    strcat(request, peer_id);
    strcat(request, "&port=");
    strcat(request, "31337");
    strcat(request, "&downloaded=");
    strcat(request, "0");
    strcat(request, "&left=");
    strcat(request, "12852");
    strcat(request, "&event=");
    strcat(request, event);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "host: ");
    strcat(request, hostname);
    strcat(request, "\r\n\r\n");*/

    free(announce);
    free(hostname);

    debug(1);

    return strlen(request);
}

void query(char request[200], char* tracker, int* sockfd)
{
    debug(3);

    int n = 0, port = 80, url_len = strlen(tracker);
    char* hostname = (char*) malloc(url_len);
    char* protocol = (char*) malloc(url_len);
    char recvbuf[1024];
    struct addrinfo hints, *res;;

    url_hostname(tracker, hostname);
    url_protocol(tracker, protocol);
    url_port(tracker, &port);

    //printf("\n\nannounce request!\nurl: %s\nhostname: %s\nannounce: %s\nprotocol: %s\nport: %d\n\n", tracker, hostname, announce, protocol, port);
    memset(&hints, 0, sizeof(hints));
    memset(recvbuf, '0',sizeof(recvbuf));
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

    debug(4);
}

//todo: read whole buffer at once, scan offset for "peers???:" and begin scanning for IP:Port.
void response(int* sockfd)
{
    int num, msglen = 1;
    char recvbuf[1024];
 
    memset(recvbuf, '0', sizeof(recvbuf));

    while ((num = read(*sockfd, recvbuf, msglen)) > 0) //sizeof(recvbuf)-1) = nbytes (1)
    {
<<<<<<< HEAD
        recvbuf[num] = 0;                   //set null char.
=======
        recvbuf[num] = 0;
        if(fputs(recvbuf, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
            return;
        }
>>>>>>> 5f7365b9218ae91dad4067b1c73412bf70aba8ce

        // strcmp(recvbuf, ":") ? msglen=5:msglen=1;
        if (strcmp(recvbuf, ":"))
            msglen = 5;
        else
            msglen = 1;

        int dls = 0;
        char terminate = '0';
        if (strcmp(recvbuf, "peers") == 0)
        {

            while (terminate != ':')    //skip length, skip : char
             read(*sockfd, &terminate, 1);

<<<<<<< HEAD
            while (1) 
            {
                 port = 0;

                printf("\nIP: ");

                 //read IP, 4 ordered bytes.
                for (i = 0; i < 4; i++)
                {
                     if ((num = read(*sockfd, &data, 1) != 1))
                        return;
                     printf("%d.", data);
                 }

                //read port:
                if ((num = read(*sockfd, &data, 1) != 1))
                    return;
                port = (data * 256) ;
                if ((num = read(*sockfd, &data, 1) != 1))
                    return;
                port += data;
                printf("\nPORT: %d\n", port);
            }   
=======

while (num > 0)
{
            int sx = 0;

            printf("\n\nIP: ");
            for (sx = 0; sx < 4; sx++)
            {
                num = read(*sockfd, &dls, 1);
                printf("%d.", dls);
            }

            //x * 2^⁸ -1
            int port = 0;
            dls = 0;
            num = read(*sockfd, &dls, 1);
            printf(" %d ", dls);

            //swap byte order
            if (dls != 0)
                port = (dls * 256) ;

            dls = 0;
            num = read(*sockfd, &dls, 1);
            printf(" %d ", dls);
            printf("\n");
            port += dls;

            printf("\nPORT: %d\n", port);
        }
>>>>>>> 5f7365b9218ae91dad4067b1c73412bf70aba8ce
        }

    } 

    if (num < 0)
    {
        printf("\n Read error \n");
        return;
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
