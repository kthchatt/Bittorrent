/* MOTD.c 
 * 2014-04-12
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Fetches MOTD from a web server running PHP and fortune. Begin the MOTD with char #.
 */

#include "MOTD.h"

 

#define MAX_RESPONSE 800
#define REPLY_TIMEOUT 2
#define HEADER '#'


pthread_t thread;

void* fetch(void* arg)
{
    char* response = (char*) arg;
    char* request = malloc(200);
    char recvbuf[MAX_RESPONSE];
    sprintf(request, "GET / HTTP/1.1\r\nhost: www.google.se\r\n\r\n");
    struct addrinfo hints, *res;
    int sockfd, num, header;

    //memset(&response, '\0', MAX_RESPONSE);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo("www.google.se", "80", &hints, &res);

     if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
    {
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
        {
            send(sockfd, request, strlen(request), 0); 
            sleep(REPLY_TIMEOUT);
            
            if ((num = read(sockfd, recvbuf, sizeof(recvbuf)-1)) > 0)
            {
                while (recvbuf[header] != HEADER && header < num)
                    header++;

                memset(response, '\0', MAX_RESPONSE);
                memcpy(response, recvbuf + header, num - header);
            }
        } 
    }
    close(sockfd);
    free(request);
    return;
}

//construct a http query
void MOTD_fetch(char* response)
{
    //create thread
    if (!(pthread_create(&thread, NULL, fetch, response)))
        printf("\nFetching the MOTD!..");
}