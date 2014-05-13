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


//for MOTD.h
#define MOTD_TIMEOUT 2
#define MOTD_MAXLEN  250

//motd is threaded. be careful with access during loading.
void MOTD_fetch(char* response);

#endif