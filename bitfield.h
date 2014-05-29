/* bitfield.c
 * 2014-05-22
 * Robin Duda
 *  Manipulating and finding differencies in bitfield.
 */

#ifndef _bitwise_h
#define _bitwise_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bool char
#define true 1
#define false 0

bool bitfield_get(char* bitfield, int index);			//returns 1 if bit set, returns 0 if bit not set.
void bitfield_set(char* bitfield, int index);			//sets bit at index to 1
void bitfield_clear(char* bitfield, int index);			//sets bit at index to 0
double bitfield_percent(char* bitfield, int length);	//get percents of bits set to 1.
char bitfield_reverse(char x);   						//reverse the bit-order in a byte. used for sending the bitfield over network.

#endif