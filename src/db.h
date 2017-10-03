#include <sqlite3.h>

extern sqlite3* db;
int db_check(int code);
void db_init(const char* name);

#define BIND(a) sqlite3_bind_ ## a

#define STEP(a) db_check(sqlite3_step(a))

#define PREPARE(b,a) sqlite3_prepare(db,a,sizeof(a)-1,&b,NULL)
