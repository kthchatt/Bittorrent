#include <openssh/sha.h>

int main(){
    char data[] = "Hello, world!";
	size_t length = sizeof(data);

	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1(data, length, hash);

	printf("%x", data[0]);
}
