#include "hook-common.h"

struct entry {
	string name;
	struct timespec mtime;
};

struct entry read_entry(int inp) {
	struct entry ret;
	smallstring_read(&ret.name, inp);
	read(inp,&ret.mtime,sizeof(ret.mtime));
	return ret;
}

static void restore_mtime(struct entry e) {
	// as

void restore(int inp) {
	enum operation op;
	ensure_eq(sizeof(op),read(inp,&op,sizeof(op)));
	switch(op) {
	case ASCEND:
		return;
	case DESCEND: {
		struct entry e = read_entry(inp);
		mkdir(e.name.s);
		ensure0(chdir(e.name.s));
		restore(inp);
		ensure0(chdir(".."));
		restore_mtime(e);
		return;
	}
	case ENTRY:
		struct entry e = read_entry(inp);
		restore_mtime(e);
	};
}
