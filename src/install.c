#define _GNU_SOURCE

#include "mystring.h"

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

	chdir(".git/hooks/");
	char temp[] = ".tmpXXXXXX";
	int out = mkstemp(temp);
	assert(out >= 0);
	write(out,here,hlen);
	write(out,LITLEN("store\n"));

	ensure0(rename(temp,"pre-commit"));
	close(out);
	out = mkstemp(temp);
	write(out,here,hlen);
	write(out,LITLEN("restore\n"));
	ensure0(rename(temp,"post-checkout"));

	symlink("post-checkout","post-merge");
	symlink("post-checkout","post-rewrite");
	symlink("post-checkout","post-reset");
	return 0;
}
	

	
	return 0;
}
