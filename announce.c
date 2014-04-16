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
#include <regex.h>


/*
domain    (\/.*-)?(w+)([a-z, A-Z, 0-9, .])*
protocol  ^[a-z, A-Z, 0-9]*
*/

void regx()
{     
    regex_t regex;
    int reti;
    char msgbuf[100];
    char* damessage = "http://www.sub.domain.tld/filespec.ext";


/* Compile regular expression */
    reti = regcomp(&regex, "^(?:\w+://)?([^/?]*)", 0);
    if( reti )
    { 
        fprintf(stderr, "Could not compile regex\n"); exit(1); 
    }

/* Execute regular expression */
    reti = regexec(&regex, "bbb", 0, NULL, 0);
    if( !reti )
    {
            puts("Match");
    }
    else if( reti == REG_NOMATCH )
    {
        puts("No match");
    }
    else
    {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }

/* Free compiled regular expression if you want to use the regex_t again */
    regfree(&regex);
}


//ip is set to 0 for non-proxy connections.
//TODO: Return list of peers.
int announce(char* tracker, char* info_hash, char* peer_id, char* ip, 
              char* event, int downloaded, int left) 
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct addrinfo hints, *res;
    char domain[100];
    char announce[100];
    int tracker_port = 0;

    //TODO: Extract from tracker.


    //TODO: Extract domain name.            (sub.domain.tld)
    //TODO: Extract port from url.          (:xx)
    //TODO: Extract announce from url.      (/trackering/tracker/announce)

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(tracker, "[tracker_port]", &hints, &res);

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    //TODO: bind to port, include port in announce request.

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    char request[200];

//TODO: Append parameters to announce.  (?info_hash=20&peer_id=20&ip=&port=&downloaded=&left=&event=)
    strcat(request, "GET");
    strcat(request, " [announceurl+parameters] ");
    strcat(request, "HTTP/1.1\r\n");
    strcat(request, "host: ");
    strcat(request, "[sub.domain.tld]");
    strcat(request, "\r\n\r\n");

    int len, bytes_sent;
    len = strlen(request);
    bytes_sent = send(sockfd, request, len, 0);

    memset(recvBuff, '0',sizeof(recvBuff));
    while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
            return 1;
        }
    } 

    //TODO: All data received, interpret peer list.

    if (n < 0)
    {
        printf("\n Read error \n");
        return 1;
    }  

    return 0;
}


//main for testing.
int main(int argc, char *argv[])
{
    /*if (announce("protocol://retracker.hq.ertelecom.ru:port/announce-url","iiiinnnnffffoooohashkkkkk", "peerIDaaaabbbbcdeeff", "0", "Started", 0, 120582) != 0)
    {
        printf("Announce Error.");
    }*/

        regx();
    

    return 0;
}