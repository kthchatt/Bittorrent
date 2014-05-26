/* MOTD.c 
 * 2014-04-12
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Fetches MOTD from a web server running PHP and fortune. Begin the MOTD with char #.
 */

#include "MOTD.h"

pthread_t thread;

void* fetch(void* arg)
{
    char* response = (char*) arg;
    char* request = malloc(128);
    char recvbuf[MOTD_MAXLEN];
    sprintf(request, "GET /MOTD.php HTTP/1.1\r\nhost: 127.0.0.1\r\n\r\n");
    struct addrinfo hints, *res;
    int sockfd, num, header = 0;

    strcpy(response, "MOTD Not Available.");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo("localhost", "80", &hints, &res);

     if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
    { 
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
        {
            send(sockfd, request, strlen(request), 0); 
            sleep(MOTD_TIMEOUT);

            if ((num = recv(sockfd, recvbuf, MOTD_MAXLEN-1, 0)) > 0)
            { 
                while (recvbuf[header] != '#' && header < num)
                {
                    header++;
                }
                header++;

                recvbuf[num-1] = '\0';
                strcpy(response, recvbuf+header); //skip final rnrn.
            }
        } 
    }

    close(sockfd);
    free(request);
    return NULL;
}

//construct a http query
void MOTD_fetch(char* response)
{
    //create thread
    if (!(pthread_create(&thread, NULL, fetch, (void*) response)))
        printf("\nFetching the MOTD!..");
}