// announce.c 
// 2014-04-16
// Robin Duda

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


//ip is set to 0 for non-proxy connections.
void announce(char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded = 0, int left = 0);
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(tracker, "http", &hints, &res);

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 

    memset(recvBuff, '0',sizeof(recvBuff));

    if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    //TODO: bind to port, include port in announce request.

    if( connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    //TODO: Build request header.
    char *request = "GET /announce?info_hash=12345678901234567890&peer_id=ABCDEFGHIJKLMNOPQRST&ip=255.255.255.255&port=6881&downloaded=1234&left=98765&event=started HTTP/1.1\r\nhost: retracker.hq.ertelecom.ru\r\n\r\n";//"GET / HTTP/1.1\r\nhost: www.google.se\r\n\r\n";
    int len, bytes_sent;

    printf("query: %s", request);
    fflush(stdout);

    len = strlen(request);
    bytes_sent = send(sockfd, request, len, 0);

    //TODO: Interpret peer list.
    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    }  
}


//main for testing.
int main(int argc, char *argv[])
{
    announce("tracker.istole.it","iiiinnnnffffoooohashkkkkk", "peerIDaaaabbbbcdeeff", "0", "Started", 0, 120582);
    return 0;
}