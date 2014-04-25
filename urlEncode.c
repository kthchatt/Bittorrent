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



char *urlEncode(char *hash)
{	
	char buffer[60];
	int i, counter, pos;
	for(i=0; i<60; i++){
		if(counter >= 2){
			buffer[i] = str[pos];
			pos++;
		}else
			buffer[i] = '%';

		counter++;
		//i%2 ? buffer[i] = '%' : buffer[i] = str[i*0.5];
	}
	
	return buffer;
	/*
    int i;
    char tbl[256];
    for(i=0; i<256; i++)
        tbl[i] = isalnum(i)||i == '~'||i == '-'||i == '.'||i == '_' ? i : 0;

    char *ptbl = tbl;

	for (; *str; str++) {
		ptbl[*str] ? sprintf(newStr, "%c", ptbl[*str]) : sprintf(newStr, "%%%02X", *str);
		while (*++newStr);
	}*/
}

