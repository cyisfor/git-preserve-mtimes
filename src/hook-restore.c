#define _GNU_SOURCE // utimensat, AT_FDCWD
#include "hook-common.h"

#include "ensure.h"

#include <fcntl.h> // AT_FDCWD
#include <sys/stat.h> // utimensat, mkdir
#include <unistd.h> // chdir

struct entry {
	string name;
	struct timespec mtime;
};

static void read_entry(struct entry* ret, int inp) {
	smallstring_read(&ret->name, inp);
	read(inp,&ret->mtime,sizeof(ret->mtime));
}

static void restore_mtime(struct entry e) {
	// assume we're in the right directory
	struct timespec times[2] = {
		e.mtime, // meh to atime
		e.mtime
	};
	utimensat(AT_FDCWD, e.name.s, times);
}

void restore(int inp) {
	enum operation op;
	struct entry e = {};
	ensure_eq(sizeof(op),read(inp,&op,sizeof(op)));
	switch(op) {
	case ASCEND:
		return;
	case DESCEND: {
		read_entry(&e, inp);
		mkdir(e.name.s);
		ensure0(chdir(e.name.s));
		restore(inp);
		ensure0(chdir(".."));
		// do this AFTER restoring its contents
		restore_mtime(e);
		return;
	}
	case ENTRY:
		struct entry e = read_entry(inp);
		restore_mtime(e);
	};
}

int main(int argc, char *argv[])
{
	int inp = open(TIMES_PATH,O_RDONLY);
	ensure_ge(inp,0);
	restore(inp);
	return 0;
}
