/* regexing.c
 * 2014-04-17
 * Robin Duda
 * Reference URL: http://www.lemoda.net/c/unix-regex/index.html
 * Takes a string and regex type, returns regex match.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

 #define ERROR_MSG 100
 #define DOMAIN 0
 #define PROTOCOL 1
 #define PORT 2

char* regtable[4] = {"(\\/.*-)?(w+)([a-z, A-Z, 0-9, .])*",  //domain
                     "^([a-z, A-Z]+)",                      //protocol
                     "PORT_REGEX",                          //port
                     "FILEPTR_REGEX"                        //fileptr 
                     };                  

//scans source for regex reg and puts the result in dest.
//dest only returns one match, change dest to array for multiple returns.
void execute_regex (regex_t* reg, char* source, char* dest)
{
    char* mpos = source;
    int match_count = 5;
    int i, k, j;
    regmatch_t result[match_count];

    while (1)
    {
        char* extract;
        int unmatched = regexec(reg, mpos, match_count, result, 0);

        if (unmatched == 1)
            break;

        printf("-%d-", unmatched);

        for (i = 0; i < match_count; i++)
        {
            int start;
            int finish;

            if (result[i].rm_so == -1)
                break;

            start = result[i].rm_so + (mpos - source);
            finish = result[i].rm_eo + (mpos - source);

            /*
                printf("$& is ");
            else
                printf("$%d is ", i);*/

            k = 0;
            for(j = result[i].rm_so; j < result[i].rm_eo; ++j) 
            {
                //printf("%c", source[j]);
                dest[k] = source[j];
                k++;
            }

            dest[k] = '\0';
           // printf("-%s-", dest);
        }
        mpos += result[0].rm_eo;
    }

    return;
}

//compile and select the correct regex string depending on type.
int regex_string(char* source, char* dest, int type)
{
    regex_t reg;
    char* query;

    //printf("source: %s, dest: %s\n", source, dest);

    switch(type)
    {
        case DOMAIN: query = regtable[DOMAIN];
            break;
        case PROTOCOL: query = regtable[PROTOCOL];
            break;
    }

    printf ("Searching for '%s' in '%s'\n", query, source);

    if (regcomp(&reg, query, REG_EXTENDED|REG_NEWLINE) != 0)
        printf("Failed to compile %s", query);

    execute_regex(&reg, source, dest);
    regfree(&reg);
}

//function caller.
int main(int argc, char ** argv)
{
    char* source;
    char* dest;

    source = "protocol://www.sub.domain.tld/fileptr";
    dest = (char*) malloc(strlen(source)+1);

    regex_string(source, dest, PROTOCOL);
    printf("source: %s, dest: %s\n", source, dest);

    return 0;
}