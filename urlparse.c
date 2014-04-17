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

   // if (*port > 0 || *port >= 65535)
   //     *port = 80;
}



void testing(char* urls)
{
    char* url;
    char* prot;
    char* host;
    int port = 40;

    url = urls;//"http://www.datgoed.domain.tld/announce";
    int len = strlen(url);
    printf("URL: %s\n", url);

    host = (char*) malloc(len);
    url_hostname(url, host);
    printf("hostname: %s\n", host);

    prot = (char*) malloc(len);
    url_protocol(url, prot);
    printf("protocol: %s\n", prot);

    url_port(url, &port);
    printf("\nport: %d\n", port);
}

//function caller.
int main(int argc, char ** argv)
{

    testing("http://tracker.istole.it:456/announce");
    testing("tracker.istole.it:801");
    testing("www.tracker.now:500/");

    return 0;
}