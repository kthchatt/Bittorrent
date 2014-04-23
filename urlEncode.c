#include "urlEncode.h"

/* Example
int main()
{
	unsigned char str[] = "http://hej hej";
	char encodedStr[sizeof(str) * 3];

	urlEncode(str, encodedStr);

	printf("%s", encodedStr);
	return 0;
}*/


void urlEncode(unsigned char *hash, char *newStr)
{
    int i;
    char tbl[256];
    for(i=0; i<256; i++)
        tbl[i] = isalnum(i)||i == '~'||i == '-'||i == '.'||i == '_' ? i : 0;

    char *ptbl = tbl;

	for (; *hash; hash++) {
		ptbl[*hash] ? sprintf(newStr, "%c", ptbl[*hash]) : sprintf(newStr, "%%%02X", *hash);
		while (*++newStr);
	}
}

