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
 #define TRUE 1
 #define FALSE 0

 /*
domain    (\/.*-)?(w+)([a-z, A-Z, 0-9, .])*
protocol  ^[a-z, A-Z, 0-9]*
*/

int compile_regex (regex_t* reg, char* regex)
{
    if (regcomp(reg, regex, REG_EXTENDED|REG_NEWLINE) != 0)
    {
        printf("regex failed to compile!");
        return 1;
    }

    return 0;
}

int execute_regex (regex_t* reg, char* string)
{
    char* match = string;
    int match_count = 5;
    regmatch_t hit[match_count];

    while (TRUE)
    {
        int i = 0;
        int unmatched = regexec(reg, match, match_count, hit, 0);

        if (unmatched)
        {
            printf("no more matches");
            return unmatched;
        }

        for (i = 0; i < match_count; i++)
        {
            int start;
            int finish;

            if (hit[i].rm_so == -1)
                break;

            start = hit[i].rm_so + (p - string);
            finish = hit[i].rm_eo + (p - string);

            if (i == 0)
                printf("$& is ");
            else
                printf("$%d is ", i);

            printf("'%.*s' (bytes %d:%d)\n", (finish - start), string + start, start, finish);
        }
        p += hit[0].rm_eo;
    }

    return 0;
}
/*
static int match_regex (regex_t * r, const char * to_match)
{
    const char * p = to_match;
    const int n_matches = 10;
    regmatch_t m[n_matches];

    while (TRUE) 
    {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) 
        {
            printf ("No more matches.\n");
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) 
        {
            int start;
            int finish;
            if (m[i].rm_so == -1) 
            {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) 
            {
                printf ("$& is ");
            }
            else 
            {
                printf ("$%d is ", i);
            }
            printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
                    to_match + start, start, finish);
        }
        p += m[0].rm_eo;
    }
    return 0;
}*/

int main(int argc, char ** argv)
{
    regex_t reg;
    const char * query;
    const char * source;

    query = "(\\/.*-)?(w+)([a-z, A-Z, 0-9, .])*";
    source = "http://www.sub.domain.tld:80/file_specifier";

    printf ("Trying to find '%s' in '%s'\n", regex_text, find_text);
    compile_regex(&reg, query);
    execute_regex(&reg, source);
    regfree (&reg);
    return 0;
}