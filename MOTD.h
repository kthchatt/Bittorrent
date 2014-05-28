 /* MOTD.c 
 * 2014-04-12
 * Robin Duda
 * Reference URL: http://beej.us/guide/bgnet/output/html/multipage/syscalls.html
 * Fetches MOTD from a web server running PHP and fortune. Begin the MOTD with char #.
 */

 #ifndef _MOTD_h
 #define _MOTD_h 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//MOTD_HOST defines the source of the motd, this may be any source printing a motd to the socket (http-header will be stripped). Use # as delimeter. #Message of the day.#
#define MOTD_TIMEOUT 2
#define MOTD_MAXLEN  250
#define MOTD_HOST "localhost"

//motd is threaded. be careful with access during its download.
void MOTD_fetch(char* response);

#endif