#include "db.h"
#include <stdbool.h>
#include <time.h> // timespec

#include "prepare.gen.h"

typedef sqlite_int64 identifier;

bool dbstuff_has_seen(identifier me);

identifier dbstuff_find(identifier parent,
							const char* name, int len);

int dbstuff_update(identifier me,
													bool isdir, struct timespec mtime);

identifier dbstuff_add(identifier parent,
							const char* name, int len, bool isdir, struct timespec mtime);

sqlite3_stmt* dbstuff_children(identifier parent);
void dbstuff_close(void);
