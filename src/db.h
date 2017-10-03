#include <sqlite3.h>

extern sqlite3* db;
int db_check(int code);
void db_init(const char* name);
