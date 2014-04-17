#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 void url_hostname(char* url, char* host);			//get hostname from url. (sub.domain.tld)
 void url_protocol(char* url, char* protocol);		//get protocol from url. (http/udp)
 void url_announce(char* url, char* announce);		//get file pointer form url. (/a_tracker/annunce.php)
 void url_port(char* url, int *port);				//get url 					 (:xx..)

#endif