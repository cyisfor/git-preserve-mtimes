#include <sqlite3.h>

extern sqlite3* db;
int db_check(int code);
void db_init(const char* name);
void db_begin(void);
void db_commit(void);
void db_close(void);
#define BIND(a) sqlite3_bind_ ## a

#define STEP(a) db_check(sqlite3_step(a))

#define PREPARE(b,a) db_check(sqlite3_prepare_v2(db,a,sizeof(a)-1,&b,NULL))
