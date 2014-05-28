/* regexing.c
 * 2014-04-17
 * Robin Duda
 * Reference URL: http://www.lemoda.net/c/unix-regex/index.html
 * Takes a string and regex type, returns regex match.
 */

#ifndef _regexing_h
#define _regexing_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//executes regex defined by type on source string, save at dest.
int regex_string(char* source, char* dest, int type);	

#endif