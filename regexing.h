#ifndef _regexing_h
#define _regexing_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//executes regex defined by type on source string, save at dest.
int regex_string(char* source, char* dest, int type);	

#endif