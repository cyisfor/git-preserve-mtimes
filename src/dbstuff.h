#include "db.h"
#include <stdbool.h>
#include <time.h> // timespec

typedef sqlite_int64 identifier;

bool dbstuff_has(identifier parent,
							const char* name, int len);

identifier dbstuff_find(identifier parent,
							const char* name, int len);

identifier dbstuff_add(identifier parent,
							const char* name, int len, bool isdir, struct timespec mtime);

sqlite3_stmt* dbstuff_children(identifier parent);
