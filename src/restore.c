#define _GNU_SOURCE // utimensat, AT_FDCWD
#include "hook-common.h"
#include "dbstuff.h"
#include "note.h"
#include "ensure.h"
#include "mystring.h"

#include <fcntl.h> // AT_FDCWD
#include <sys/stat.h> // utimensat, mkdir
#include <unistd.h> // chdir

static void restore_mtime(const char* name, struct timespec mtime) {
	// assume we're in the right directory
	struct timespec times[2] = {
		mtime, // meh to atime
		mtime
	};
	//INFO("T %d",e.mtime.tv_sec);
	utimensat(AT_FDCWD, name, times, 0);
}

void restore() {
	void down(struct entry* cur) {
		while(cur) {
			if(cur->children) {
				//isdir
				ensure0(chdir(cur->name));
				down(cur->children);
				ensure0(chdir(".."));
			}
			restore_mtime(cur->name,cur->modified);
			cur = cur->next;
		}
	}
	down(dbstuff_root);
}

int main(int argc, char *argv[])
{
	note_init();
	dbstuff_open(TIMES_PATH);
	restore();
	return 0;
}
