#include "db.h"
typedef sqlite_int64 identifier;

identifier add(identifier parent,
							const char* name, int len, struct timespec mtime);

sqlite3_stmt* children(identifier parent);
