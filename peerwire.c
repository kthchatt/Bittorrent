 //entry point in tracker threads. (functions will be threaded too.)


#include "peerwire.h"


//4 byte message length
//1 byte message type
//4 byte piece index

void main(void)
{
    int n = 0, port = 80, sockfd, url_len = 200, i;
    char* hostname = "1.1.1.1";
    struct addrinfo hints, *res;
    char request[9];
    int len = 5;
    int index = 45;
    memset(request, 0, sizeof(request));
    
    request[3] = 5; //len
    request[4] = 4;
    request[8] = 6;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(hostname, "80", &hints, &res);

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) > -1)
    {
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) > -1)
        {
            send(sockfd, request, 9, 0);
        } 
    }
}

//
