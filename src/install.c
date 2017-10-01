#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	// may be relative, but ok
	const char* here = argv[0];
	size_t len = strlen(here);
	size_t hlen = 0;
	char* s = memrchr(here,'/',len);
	if(s) {
		hlen = s - here + 1; // include the slash
	}
	fwrite(here,hlen,1,stdout);
	putchar('\n');
	puts(here);
	
	return 0;
}
