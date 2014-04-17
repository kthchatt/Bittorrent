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

 /*
domain    (\/.*-)?(w+)([a-z, A-Z, 0-9, .])*
protocol  ^[a-z, A-Z, 0-9]*
*/

char* regtable[2] = {"(\\/.*-)?(w+)([a-z, A-Z, 0-9, .])*",  //domain
                     "^[a-z, A-Z, 0-9]*"};                  //protocol

void execute_regex (regex_t* reg, char* source, char* dest)
{
    char* mpos = source;
    int match_count = 5;
    regmatch_t result[match_count];

    while (1)
    {
        int i = 0, k;
        char* extract;
        int unmatched = regexec(reg, mpos, match_count, result, 0);

        if (unmatched)
            return;

        for (i = 0; i < match_count; i++)
        {
            int start;
            int finish;

            if (result[i].rm_so == -1)
                break;

            start = result[i].rm_so + (mpos - source);
            finish = result[i].rm_eo + (mpos - source);

            if (i == 0)
                printf("$& is ");
            else
                printf("$%d is ", i);

            printf("'%.*s' (bytes %d:%d)\n", (finish - start), (source + start), start, finish);

            int j, k = 0;
            //dest = (char*) malloc(result[i].rm_eo - result[i].rm_so);

            for(j = result[i].rm_so; j < result[i].rm_eo; ++j) 
            {
                printf("%c", source[j]);
                dest[k] = source[j];
                k++;
            }

            dest[k] = '\0';

            printf("-%s-", dest);
           /* dest = (char*) malloc(result[i].rm_eo - result[i].rm_so);
            strncpy(dest, &source[result[i].rm_so], result[i].rm_eo - result[i].rm_so);

            printf("Result: -%s-\n", dest);
            printf("Result2: -%s-\n", source);*/

        }
        mpos += result[0].rm_eo;
    }

    return;
}

int regex_string(char* source, char* dest, int type)
{
    regex_t reg;
    char* query;

    printf("source: %s, dest: %s\n", source, dest);

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

int main(int argc, char ** argv)
{
    char* source;
    char* dest;

    source = "protocol://www.sub.domain.tld/fileptr";
    dest = (char*) malloc(strlen(source)+1);

    regex_string(source, dest, DOMAIN);
    printf("source: %s, dest: %s", source, dest);

    return 0;
}