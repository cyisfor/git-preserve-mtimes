#include <stdbool.h>
#include <time.h> // timespec

struct entry {
	struct entry* children;
	struct entry* parent;
	struct entry* next;
	const char* name;
	int namelen;
	struct timespec modified;
	bool was_seen;
};

struct entry* dbstuff_find(struct entry* parent,
													 const char* name, int len);

int dbstuff_update(identifier me,
													bool isdir, struct timespec mtime);

identifier dbstuff_add(identifier parent,
							const char* name, int len, bool isdir, struct timespec mtime);

sqlite3_stmt* dbstuff_children(identifier parent);
void dbstuff_close(void);
