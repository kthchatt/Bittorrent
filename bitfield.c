/* bitwise.c
 * 2014-05-22
 * Robin Duda
 *  Manipulating and finding differencies in bitfield.
 */

#include "bitfield.h"

//returns 1 if bit set, returns 0 if bit not set.
bool bitfield_get(char* bitfield, int index)
{
	return (1<<(index%8) & bitfield[index/8]) > 0 ? 1 : 0;
}

void bitfield_set(char* bitfield, int index)
{
	bitfield[index/8] |= (1<<(index%8) | bitfield[index/8]);
}

void bitfield_clear(char* bitfield, int index)
{
	if (bitfield_get(bitfield, index) == 1)
		bitfield[index/8] = (~(1<<(index%8)) & bitfield[index/8]);
}

double bitfield_percent(char* bitfield, int length)
{
	int i;
	double percent = 0.0;

	for (i = 0; i < length; i++)
		if (bitfield_get(bitfield, i) == 1)
			percent += (100 / length);

	return percent;
}

/*
int main()
{
	char* bitfield = malloc(8);
	int i;

	setbit(bitfield, 7);
	setbit(bitfield, 0);
	setbit(bitfield, 63);
	setbit(bitfield, 8);
	setbit(bitfield, 9);

	clearbit(bitfield, 7);
	clearbit(bitfield, 8);

	for (i = 0; i < 64; i++)
		printf("\n%d = [%d]", i, getbit(bitfield, i));

	return 0;
}*/