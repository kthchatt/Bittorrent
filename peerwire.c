 //entry point in tracker threads. (functions will be threaded too.)


#include "peerwire.h"

/*
	PIECE INDEXES ARE ZERO-BASED
	BLOCK SPECIFIES BYTE OFFSET IN PIECE
	DROP PACKETS WITH INVALID LENGTH

	MESSAGE HEADER
		4 BYTES LENGHT SPECIFIER
		1 BYTE MESSAGE TYPE
		4 BYTE INDEX | VARIABLE SIZE PAYLOAD

	NON-PAYLOAD MESSAGES
		choke: 			<len=0001><id=0> 						
		unchoke: 		<len=0001><id=1>						
		interested: 	<len=0001><id=2>					.
		not interested: <len=0001><id=3>				
		have: 			<len=0005><id=4><piece index>

	PAYLOAD MESSAGES
		piece: 	<len=0009+X><id=7><index><begin><block> 
		cancel: <len=0013>  <id=8><index><begin><length>		
		port: 	<len=0003>  <id=9><listen-port>
*/

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
