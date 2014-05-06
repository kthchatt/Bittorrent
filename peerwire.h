 #ifndef _peerwire_h
 #define _peerwire_h

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

//create a peerwire connection using tcp.
void* peerwire_thread_tcp(peer_t* peer, char info_hash[21], char peer_id[21]);
//create a peerwire connection usind udp.
void* peerwire_thread_udp(peer_t* peer);


#endif