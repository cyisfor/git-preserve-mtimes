/* hokay, so strategy...
	annoying as **** to try and merge index and working tree, so let sqlite figure it out.

	a
	  b
		c
		  d
			e
		f
		  g

  one-to-many or parent-child relationship, but also a next entry.

	if git gives us entries b, c, and f, then c->next will be b, and f->next will be c
	
*/

#include "prepare.gen.c"

identifer add(identifier parent,
							const char* name, int len, struct timespec mtime) {
	BIND(text)(add_find, 1, name, len, NULL);
	BIND(int64)(add_find, 2, parent);
	int res = STEP(find);
	if(res == SQLITE_ROW) {
		identifier ret = sqlite3_column_int64(add_find, 0);
		sqlite3_reset(add_find);
		return ret;
	}
		
	sqlite3_reset(add_find);
	BIND(text)(add_insert, 1, name, len, NULL);
	BIND(int64)(add_insert, 2, identifier);
	BIND(int64)(add_insert, 3, mtime.tv_sec);
	BIND(int64)(add_insert, 4, mtime.tv_nsec);
	STEP(add_insert);
	return sqlite3_last_insert_rowid(db);
}

/* expose this, because... tail recursion gets messed up when you pass function pointers. */

sqlite3_stmt* children(identifier parent) {
	// this must be reentrant! we will call children, before resetting children, when recursing!
	sqlite3_stmt* stmt;
	char buf[0x100];
	snprintf(buf,0x100,"CREATE TEMPORARY TABLE IF NOT EXISTS children%d AS SELECT id,isdir,name,modified,modifiedns FROM entries WHERE parent = ?", parent);
	PREPARE(stmt,buf);
	BIND(int64)(stmt,1,parent);
	STEP(stmt); // if not created, then okay.
	sqlite3_finalize(stmt);
	// now get results
	snprintf(buf,0x100,"SELECT * FROM children%d",parent);
	PREPARE(stmt,buf);
	return stmt;
}

// note: children(0) -> topmost entries
