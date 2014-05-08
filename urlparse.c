/* urlparse.c
 * 2014-04-17
 * Robin Duda
 *   Read data from URL.
 *      All methods are thread safe. 
 *      The caller must allocate memory.
 */

#include "urlparse.h"

//returns file path, fx: http://www.tracker.domain.com:port/sft/tracker/announce.php
 //will return /sft/tracker/, used to replace filepointer for scrape etc.
 void url_path(char* url, char* path)
 {
    int i, path_start = 0, path_end, len;

    len = strlen(url);
    memset(path, '\0', len);

    for (i = 7; i < len; i++)
    {
        if (url[i] == '/' && path_start == 0)   //find first occurrence of /
            path_start = i;
    
        if (url[i] == '/')                      //find most recent occurence of /
            path_end = i;
    }

    strncpy(path, url+path_start, path_end-path_start);
}


//extract hostname from url. 
void url_hostname(char* url, char* host)
{
    int i, pos = 0, domain_start = 0, domain_end, len;

    len = strlen(url);
    memset(host, '\0', len);
    domain_end = len;

    for (i = 0; i < len; i++)
    {
        if (i > 6 && (url[i] == '/' || url[i] == ':'))
        {
            domain_end = i;
            break;
        }

        if (url[i] == ':' && domain_start == 0)
        {
            domain_start = i+3;
            i += 3;
        }
    }

    for (i = domain_start; i < domain_end; i++)
    {
        host[pos] = url[i];
        pos++;
    }
}

//extract protocol from url.
void url_protocol(char* url, char* protocol)
{
    int i = 0, len, protocol_end = 0, k = 0;
    
    len = strlen(url);
    memset(protocol, '\0', len);

    for (i = 0; i < len; i++)
    {
        if (url[i] == ':')
        {
            protocol_end = i;
            break;
        }
    }

    for (i = 0; i < protocol_end; i++)
    {
        protocol[k] = url[i];
        k++;
    }

    //if protocol omitted default to http (https not supported, will default to http)
    if (protocol_end > 4 || protocol_end == 0)
    {
        strcpy(protocol, "http");
    }
}

//finds the port number, searches from end of string to first colon.
void url_port(char* url, int *port)
{
    int i, len, port_end, port_start, occur = 0, amp = 0;
    *port = 0;

    len = strlen(url);
    port_end = len;

    for (i = len; i > 0; i--)
    {
        if ((url[i] < 48 || url[i] > 57) && url[i] != ':')
        {
            port_end = i-1;
        }

        if (url[i] == ':')
        {
            port_start = i;
            break;
        }
    }

    amp = 1;
    for (i = port_end; i > port_start; i--)
    {
        *port += (url[i]-48) * amp;
        amp *= 10;
    }

    if (*port < 1)
        *port = 80;
}

//extracts the announce url from url, "http://tracker:80/announce.php" = /announce.php
void url_announce(char* url, char* announce)
{
    int len, i, j = 0, announce_started = 0;

    len = strlen(url);
    memset(announce, '\0', len);

    if (len < 6)
        return;

    for (i = 7; i < len; i++)
    {
        if (url[i] == '/' || announce_started == 1)
        {
            announce[j] = url[i];
            announce_started = 1;
            j++;
        }
    }
}


//extracts a bencoded value dictionary-wise.
 int bdecode_value(char* source, char* search)
 {
    int i, intlen = 0, value = 0;
    int source_len = strlen(source);
    int search_len = strlen(search);
    char* seek = (char*) malloc(search_len);
    char tmp[5];

    memset(tmp, '\0', sizeof(tmp));
    memset(seek, '\0', search_len);
    
    for (i = 0; i < source_len; i++)
    {
        strncpy(seek, source+i, search_len);
        seek[search_len] = '\0';

        if (strcmp(seek, search) == 0)
        {   
            i += search_len+1;
            while (i < source_len && intlen < 5 && 47 < source[i] && source[i] < 58)    //while is digit read.
            {
                tmp[intlen] = source[i]; 
                intlen++;
                i++;
            }
            value = atoi(tmp);                          //safe to use here.
            break;
        }
    }
    free(seek);
    return value;
 }

 //output = 60+nullterm
void url_encode(char* hash, char* output)
{
    memset(output, '\0', 61);

    int i, k = 1;
    for (i = 0; i < 20; i++)
    { 
        strcat(output, "%");
        snprintf(output+k, 61, "%x", (unsigned char) hash[i]);
        k += 3;
    }
    output[60] = '\0';
}


/*------------------------------- BEGIN: REMOVE WHEN COMPLETE ----------------------------------*/
/*void testing(char* urls)
{
    char* url;
    char* prot;
    char* host;
    char* announce;
    char* path;
    int port = 40;

    url = urls;//"http://www.datgoed.domain.tld/announce";
    int len = strlen(url);
    printf("URL: %s\n", url);

    announce = (char*) malloc(len);
    url_announce(url, announce);
    printf("announce: %s\n", announce);

    host = (char*) malloc(len);
    url_hostname(url, host);
    printf("hostname: %s\n", host);

    prot = (char*) malloc(len);
    url_protocol(url, prot);
    printf("protocol: %s\n", prot);

    path = (char*) malloc(len);
    url_path(url, path);
    printf("path: %s\n", path);

    url_port(url, &port);
    printf("port: %d\n\n", port);
}


//function caller.
int main(int argc, char ** argv)
{

    testing("http://tracker.istole.it:456/tracker5/announce");
    testing("tracker.istole5.it:801/announce");
    testing("www.tracker.now/announce/announce5");
    testing("http://192.168.0.14/announce");
    testing("udp://192.168.014/announce");
    testing("http://10.10.10.10:50/tracker/announce");
    testing("http://94.228.192.98/announce");

    return 0;
}*/

/*------------------------------- END: REMOVE WHEN COMPLETE ----------------------------------*/