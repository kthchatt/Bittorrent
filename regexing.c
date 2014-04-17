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

void execute_regex (regex_t* reg, char* string)
{
    char* mpos = string;
    int match_count = 5;
    regmatch_t result[match_count];

    while (1)
    {
        int i = 0;
        int unmatched = regexec(reg, mpos, match_count, result, 0);

        if (unmatched)
            return;

        for (i = 0; i < match_count; i++)
        {
            int start;
            int finish;

            if (result[i].rm_so == -1)
                break;

            start = result[i].rm_so + (mpos - string);
            finish = result[i].rm_eo + (mpos - string);

            if (i == 0)
                printf("$& is ");
            else
                printf("$%d is ", i);

            printf("'%.*s' (bytes %d:%d)\n", (finish - start), string + start, start, finish);
        }
        mpos += result[0].rm_eo;
    }

    return;
}

int regex_string(char* source, char* dest, int type)
{
    regex_t reg;
    char* query;

    switch(type)
    {
        case DOMAIN: query = regtable[DOMAIN];
            break;
        case PROTOCOL: query = regtable[PROTOCOL];
            break;
        case default: query = null;
            break;
    }

    //compile
    //execute
}

int main(int argc, char ** argv)
{

    query = "(\\/.*-)?(w+)([a-z, A-Z, 0-9, .])*";
    source = "http://www.sub.domain.tld:80/file_specifier";

    printf ("Searching for '%s' in '%s'\n", query, source);

    if (regcomp(&reg, query, REG_EXTENDED|REG_NEWLINE) != 0)
    {
        printf("regex failed to compile!");
        return 1;
    }
    else
        execute_regex(&reg, source);

    regfree (&reg);
    return 0;
}