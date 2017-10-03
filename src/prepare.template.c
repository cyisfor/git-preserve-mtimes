sqlite3_stmt* %1$;
sqlite3_stmt* %1$;
sqlite3_stmt* %1$;
sqlite3_stmt* %1$;

int auto_init(void) {
	db_check(sqlite3_prepare(db,%2$,%3$, &%1$, NULL));
		db_check(sqlite3_prepare(db,%2$,%3$, &%1$, NULL));
			db_check(sqlite3_prepare(db,%2$,%3$, &%1$, NULL));
			...
}

int auto_finalize(void) {
	db_check(sqlite3_finalize(%1$));
		db_check(sqlite3_finalize(%1$));
			db_check(sqlite3_finalize(%1$));
				db_check(sqlite3_finalize(%1$));
				...;
}

