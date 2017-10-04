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
	db_check(sqlite3_extended_result_codes(db,1));

	PREPARE(begin,"BEGIN");
	PREPARE(commit,"COMMIT");
#include "db.sql.gen.c"
	db_begin();
	sqlite3_exec(db, db_sql, errorderp, NULL, NULL);
	db_commit();
}

static
int tlevel = 0;

void db_begin(void) {
	if(tlevel == 0) {
		db_check(sqlite3_step(begin));
		sqlite3_reset(begin);
	}
	++tlevel;
}

void db_commit(void) {
	if(tlevel == 0) return;
	if(--tlevel == 0) {
		db_check(sqlite3_step(commit));
		sqlite3_reset(commit);
	}
}

void db_close(void) {
	if(tlevel > 0) {
		db_check(sqlite3_step(commit));
	}
	db_check(sqlite3_finalize(begin));
	db_check(sqlite3_finalize(commit));
	db_check(sqlite3_close(db));
	// always exit right after this!
}
