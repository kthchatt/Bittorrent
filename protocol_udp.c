/* protocol_udp.c
 * 2014-05-30
 * Robin Duda
 *  UDP Tracker protocol.
 */

#include "protocol_udp.h"

//get connection id
static int connect_tracker(int* sockfd, long int* connection_id)
{
    char* packet = malloc(16);
    int action = htonl(0), num;
    int transaction_id = htonl(8388608);

    memcpy(packet, connection_id, 8);   
    memcpy(packet + 8, &action, 4);  
    memcpy(packet + 12, &transaction_id, 4); 

    if (send(*sockfd, packet, 16, 0) == 16)
    {  
        if ((num = recv(*sockfd, packet, 16, 0)) == 16)
        {
            memcpy(&action, packet, 4);   
            memcpy(&transaction_id, packet + 4, 4);  
            memcpy(connection_id, packet + 8, 8);  
        }  
    }
    free(packet);
    return htonl(action);
}

//send query
static void send_scrape(scrape_t* scrape, int* sockfd, long int connection_id)
{
    int action = htonl(2), num;
    int transaction_id = htonl(8388609);
    int action_reply, transaction_reply;
    int seeders, completed, leechers;
    char* packet = malloc(36);
    char* response = malloc(20);

    memcpy(packet, &connection_id, 8);
    memcpy(packet + 8, &action, 4);
    memcpy(packet + 12, &transaction_id, 4);
    memcpy(packet + 16, scrape->swarm->info_hash, 20);
    netstat_update(OUTPUT, 36, scrape->swarm->info_hash);

    if ((send(*sockfd, packet, 36, 0)) == 36)
    {
        if ((num = recv(*sockfd, response, 20, 0)) == 20)
        {
            memcpy(&action_reply, response, 4);
            memcpy(&transaction_reply, response + 4, 4);
            memcpy(&seeders, response + 8, 4);
            memcpy(&completed, response + 12, 4);
            memcpy(&leechers, response + 16, 4);

            printf("\nReceived \tAction : %d, transaction : %d, seeders %d", ntohl(action_reply), ntohl(transaction_reply), ntohl(seeders)); fflush(stdout);
        
            if (transaction_id == transaction_reply && action == action_reply)
            {
                scrape->tracker->completed  = ntohl(seeders);
                scrape->tracker->downloaded = ntohl(completed);
                scrape->tracker->incomplete = ntohl(leechers);

                //rather underestimate the peer count, most peers will be connected to every tracker in announce-list.
                if (scrape->tracker->completed > scrape->swarm->completed)
                    scrape->swarm->completed = scrape->tracker->completed;
                if (scrape->tracker->incomplete > scrape->swarm->incomplete)
                    scrape->swarm->incomplete = scrape->tracker->incomplete;
            }
        }
        netstat_update(INPUT, num, scrape->swarm->info_hash);
    }

    free(packet);
    free(response);
}

static void receive_announce(announce_t* announce, int* sockfd, long int connection_id, int transaction_id)
{
    int num, action, i = 0, j;
    char* packet = malloc(2048);
    int interval, leechers, seeders, transaction_reply;
    char ip[4];
    unsigned short int port;
    unsigned char data;

    if ((num = recv(*sockfd, packet, 2048, 0)) > 0)
    {
        netstat_update(INPUT, num, announce->swarm->info_hash);
        printf("\nReading announce reply of %d bytes!", num);

        memcpy(&action, packet, 4);
        memcpy(&transaction_reply, packet + 4, 4);
        memcpy(&interval, packet + 8,  4);
        memcpy(&leechers, packet + 12, 4);
        memcpy(&seeders, packet + 16,  4);
        i = 20;

        //read in peers!
        lock(&announce->swarm->peerlock);
        while (i + 6 <= num && announce->swarm->peercount < MAX_SWARM_SIZE)
        {
            for (j = 0; j < 4; j++)
                ip[j] = packet[i+j];

            sprintf(announce->swarm->peer[announce->swarm->peercount].ip, "%hd.%hd.%hd.%hd", 
                (unsigned char) ip[0], (unsigned char) ip[1], (unsigned char) ip[2], (unsigned char) ip[3]);

            i += 4;
            data = packet[i];
            port = 0;
            port =  (unsigned char) data << 8;
            port += (unsigned char) packet[i+1];
            i += 2;

            sprintf(announce->swarm->peer[announce->swarm->peercount].port, "%d", (unsigned) port);

            printf("\nPeer = [%s:%s]", announce->swarm->peer[announce->swarm->peercount].ip, announce->swarm->peer[announce->swarm->peercount].port);
            announce->swarm->peercount++;
        }
        unlock(&announce->swarm->peerlock);
    }

    free(packet);
}

static void send_announce(announce_t* announce, int* sockfd, long int connection_id)
{
    char* packet = malloc(98);
    int action = htonl(1);
    int transaction_id = htonl(80080021);
    int event = htonl(2), ip = htonl(0), key = htonl(0), numwant = htonl(-1);
    short int port = htonl(announce->swarm->listenport);
    long int downloaded = htonl(0), left = htonl(10000), uploaded = htonl(0);

    memcpy(packet, &connection_id, 8);
    memcpy(packet + 8, &action, 4);
    memcpy(packet + 12, &transaction_id, 4);
    memcpy(packet + 16, announce->swarm->info_hash, 20);
    memcpy(packet + 36, announce->swarm->peer_id, 20);
    memcpy(packet + 56, &downloaded, 8);
    memcpy(packet + 64, &left, 8);
    memcpy(packet + 72, &uploaded, 8);
    memcpy(packet + 80, &event, 4);
    memcpy(packet + 84, &ip, 4);
    memcpy(packet + 88, &key, 4);
    memcpy(packet + 92, &numwant, 4);
    memcpy(packet + 96, &port, 2);

    send(*sockfd, packet, 98, 0);
    netstat_update(INPUT, 98, announce->swarm->info_hash);
    free(packet);
    receive_announce(announce, sockfd, connection_id, transaction_id);
}

void udp_announce(announce_t* announce)
{
    int sockfd = -1, port;
    struct addrinfo hints, *servinfo, *intf;
    char* hostname = malloc(MAX_URL_LEN);
    char* portname = malloc(6);
    long int connection_id =  UDP_CONNECTION_ID;

    url_hostname(announce->tracker->url, hostname);
    url_port(announce->tracker->url, &port);
    sprintf(portname, "%d", (unsigned int) port);

    printf("\nDNS: hostname = %s, portname = %s", hostname, portname);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname, portname, &hints, &servinfo) == 0)
    {
        for (intf = servinfo; intf != NULL; intf = intf->ai_next)
        {
            if ((sockfd = socket(intf->ai_family, intf->ai_socktype, intf->ai_protocol)) != -1)
                break;
        }

        if (sockfd != -1 || intf != NULL)
        {
            connect(sockfd, intf->ai_addr, intf->ai_addrlen);

            if ((connect_tracker(&sockfd, &connection_id)) == 0)
                send_announce(announce, &sockfd, connection_id);
            else
                printf("\nUDP Could not connect to %s.", announce->tracker->url);
        }
    }
        else
            printf("DNS failed for %s", announce->tracker->url);

    free(hostname);
    free(portname);
}

//scrape an UDP tracker
void udp_scrape(scrape_t* scrape)
{
    int sockfd = -1, port;
    struct addrinfo hints, *servinfo, *intf;
    char* hostname = malloc(MAX_URL_LEN);
    char* portname = malloc(6);
    long int connection_id =  UDP_CONNECTION_ID;

    url_hostname(scrape->tracker->url, hostname);
    url_port(scrape->tracker->url, &port);
    sprintf(portname, "%d", (unsigned int) port);

    printf("\nDNS: hostname = %s, portname = %s", hostname, portname);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    if (getaddrinfo(hostname, portname, &hints, &servinfo) == 0)
    {
        for (intf = servinfo; intf != NULL; intf = intf->ai_next)
        {
            if ((sockfd = socket(intf->ai_family, intf->ai_socktype, intf->ai_protocol)) != -1)
                break;
        }

        if (sockfd != -1 || intf != NULL)
        {
            connect(sockfd, intf->ai_addr, intf->ai_addrlen);

            if (connect_tracker(&sockfd, &connection_id) == 0)
                send_scrape(scrape, &sockfd, connection_id);
            else
                printf("\nUDP Could not connect to %s", scrape->tracker->url);
        }
    }
        else 
            printf("\nDNS failed for %s", scrape->tracker->url);

    free(hostname);
    free(portname);
}