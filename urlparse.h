#ifndef _urlparse_h
#define _urlparse_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 void url_hostname(char* url, char* host);			//get hostname from url. (sub.domain.tld)
 void url_protocol(char* url, char* protocol);		//get protocol from url. (http/udp)
 void url_announce(char* url, char* announce);		//get file pointer form url. (/a_tracker/annunce.php)
 void url_port(char* url, int *port);				//get url 					 (:xx..)
 void url_path(char* url, char* path);				//extract path from url.
 int bdecode_value(char* source, char* search);		//extract value from bencode string.
 void url_encode(char* hash, char* output)			//url-encode hash. output = 60.

#endif