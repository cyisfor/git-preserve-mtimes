#define _GNU_SOURCE

#include "mystring.h"
#include "ensure.h"

#include <sys/stat.h> // chmod

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
		// append, so we don't squash other pre-commit scripts
		// note: this fails hard if pre-commit is an executable
		// TODO: rename pre-commit to old-pre-commit, and have pre-commit exec it
		// but how to stop from wrapping our pre-commit, that wraps another pre-commit?
		int out = open(dest,O_APPEND|O_CREAT|O_WRONLY,0755);
		assert(out >= 0);
		ensure_eq(hlen, write(out,here,hlen));
		ensure_eq(clen, write(out,contents,clen));
		close(out);
	}
	one("pre-commit",LITLEN("store\n"));
	one("post-checkout",LITLEN("restore\n"));
	symlink("post-checkout","post-merge");
	symlink("post-checkout","post-rewrite");
	symlink("post-checkout","post-reset");
	return 0;
}
