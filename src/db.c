#include "db.h"
#include <stdlib.h> // NULL
#include <unistd.h> // write
#include <string.h>

#define LITLEN(a) a,(sizeof(a)-1)

sqlite3* db = NULL;

int db_check(int code) {
	switch(code) {
	case SQLITE_DONE:
	case SQLITE_OK:
	case SQLITE_ROW:
		return code;
	default:
		write(2,LITLEN("DB error"));
		if(db) {
			write(2,LITLEN(": "));
			write(2,sqlite3_errmsg(db),strlen(sqlite3_errmsg(db)));
		}
		write(2,LITLEN("\n"));
		abort();
	};
	return code;
}

void db_init(const char* name) {
	db_check(sqlite3_open(name, &db));
	
#include "db.sql.gen.c"
	db_check(sqlite3_exec(db, db_sql, NULL, NULL, NULL));
}
