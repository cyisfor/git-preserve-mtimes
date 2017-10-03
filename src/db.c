#include "ensure.h"
#include "db.h"
#include <stdlib.h> // NULL
#include <unistd.h> // write
#include <string.h>

#define LITLEN(a) a,(sizeof(a)-1)

sqlite3* db = NULL;

sqlite3_stmt* begin, *commit;

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

static
int errorderp(void* udata, int args, char**argv, char** colname) {
	int i;
	for(i=0;i<args;++i) {
		write(2,argv[i],strlen(argv[i]));
		write(2,LITLEN("\n"));
	}
	abort();
}

void db_init(const char* name) {
	db_check(sqlite3_open(name, &db));
	PREPARE(begin,"BEGIN");
	PREPARE(commit,"COMMIT");
#include "db.sql.gen.c"
	sqlite3_exec(db, db_sql, errorderp, NULL, NULL);
}
