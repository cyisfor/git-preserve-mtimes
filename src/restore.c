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
	void down(int parent) {
		sqlite3_stmt* children = dbstuff_children(parent);
		for(;;) {
			if(STEP(children) == SQLITE_DONE) break;
			const char* name = sqlite3_column_text(children,2);
			if(sqlite3_column_int(children,1) != 0) {
				//isdir
				ensure0(chdir(name));
				down(sqlite3_column_int64(children,0));
				ensure0(chdir(".."));
			}
			struct timespec mtime = {
				sqlite3_column_int64(children,3),
				sqlite3_column_int64(children,4)
			};
			restore_mtime(name,mtime);
		}
		sqlite3_reset(children);
	}
	down(0);
}

int main(int argc, char *argv[])
{
	note_init();
	db_init(TIMES_PATH);
	restore();
	return 0;
}
