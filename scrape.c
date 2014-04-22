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


struct 
{

} scrapedata;

//construct a http query
int build(char request[200], char* tracker, char* info_hash) 
{
    char* path = (char*) malloc(strlen(tracker));
    char* hostname = (char*) malloc(strlen(tracker));
    int port = rand()%9999+500;    //bound port: listener for info_hash.

    url_hostname(tracker, hostname);
    url_path(tracker, path);

    sprintf(request, "GET %s/scrape HTTP/1.1\r\nhost: %s\r\n\r\n", 
                                    path, hostname);
    free(path);
    free(hostname);

    return strlen(request);
}

void query(char request[200], char* tracker, int* sockfd)
{
	int   port = 80, url_len = 200;
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

void response(int* sockfd)
{

}

 void scrape(char* tracker, char* info_hash)
 {
    int sockfd,port = 80;
    char request[200];

    build(request, tracker, info_hash);
    query(request, tracker, &sockfd);
    response(&sockfd);
 }