#ifndef _bitwise_h
#define _bitwise_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool char
#define true 1
#define false 0

//returns 1 if bit set, returns 0 if bit not set.
bool bitfield_get(char* bitfield, int index);
//sets bit to 1
void bitfield_set(char* bitfield, int index);
//sets bit to 0
void bitfield_clear(char* bitfield, int index);

#endif