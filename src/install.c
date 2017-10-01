#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	// may be relative, but ok
	const char* exe = argv[0];
	size_t elen = strlen(exe);
	char* s = memrchr(exe,'/',elen);
	if(s) {
		printf("um %p %p %x\n",s,exe,s-exe);
		*s = '\0';
	}
	puts(exe);
	
	return 0;
}
