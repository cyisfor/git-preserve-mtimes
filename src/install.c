#define _GNU_SOURCE

#include "mystring.h"
#include "ensure.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h> // chdir, write etc
#include <fcntl.h> // open
#include <assert.h>
#include <stdio.h> // rename


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
	int one(const char* dest, const char* contents, size_t clen) {
		char temp[] = ".tmpXXXXXX";
		int out = mkstemp(temp);
		assert(out >= 0);
		write(out,here,hlen);
		write(out,contents,clen);

		ensure0(rename(temp,dest));
		close(out);
	}
	one("pre-commit",LITLEN("store\n"));
	one("post-checkout",LITLEN("restore\n"));

	symlink("post-checkout","post-merge");
	symlink("post-checkout","post-rewrite");
	symlink("post-checkout","post-reset");
	return 0;
}
