/* urlparse.c
 * 2014-04-17
 * Robin Duda
 *   Read data from URL.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>


//extract hostname from url. 
void url_hostname(char* url, char* host)
{
    int i, pos = 0, domain_start = 0, domain_end, len;

    len = strlen(url);
    domain_end = len;

    for (i = 0; i < len; i++)
    {
        if (i > 5 && url[i] == '/' || url[i] == ':')
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

    //if protocol omitted.
    if (domain_start > 4)
    {
        domain_start = 0;
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

    //if protocol omitted default to http
    if (protocol_end > 4)
    {
        strcpy(protocol, "http");
    }
}

//finds the port number, searches from end of string to first colon.
void url_port(char* url, int port)
{
    int i, len, port_end, port_start, occur = 0, amp = 0;
    port = 0;

    printf("port: %d %d %d", *port, port, &port);
    fflush(stdout);

    len = strlen(url);
    port_end = len;

    for (i = len; i < len; i--)
    {
    printf("[%d]", i);
    fflush(stdout);

        if (url[i] == ':')
        {
            port_start = i;
            break;
        }

        if (url[i] < 47 || url[i] > 57)
        {
            port_end = i-1;
        }
    }

    printf("CALCEN");
    fflush(stdout);

    amp = 1;
    for (i = port_end; i > port_start; i--)
    {
        printf("port [%d]\n", *port);
        port += (url[i]-48) * amp;
        amp *= 10;
    }
}



//function caller.
int main(int argc, char ** argv)
{
    char* url;
    char* prot;
    char* host;
    int port = 40;

    url = "www.datgoed.domain.tld:80/announce";
    int len = strlen(url);


    host = (char*) malloc(len);
    url_hostname(url, host);
    printf("hostname: %s\n", host);

    prot = (char*) malloc(len);
    url_protocol(url, prot);
    printf("protocol: %s\n", prot);


    url_port(url, &port);
    printf("port: %d", port);


    return 0;
}