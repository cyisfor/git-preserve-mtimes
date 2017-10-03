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

identifer add(identifier parent,
							const char* name, int len, struct timespec mtime) {
	DECLARE_STMT(find,"SELECT id FROM entries WHERE name = ? AND parent = ?)");
	DECLARE_STMT(insert,"INSERT INTO entries (name, parent, modified, modifiedns) VALUES (?,?,?,?)");
	BIND(text)(find, 1, name, len, NULL);
	BIND(int64)(find, 2, parent);
	int res = STEP(find);
	if(res == SQLITE_ROW) {
		identifier ret = sqlite3_column_int64(find, 0);
		sqlite3_reset(find);
		return ret;
	}
		
	sqlite3_reset(find);
	BIND(text)(insert, 1, name, len, NULL);
	BIND(int64)(insert, 2, identifier);
	BIND(int64)(insert, 3, mtime.tv_sec);
	BIND(int64)(insert, 4, mtime.tv_nsec);
	STEP(insert);
	return sqlite3_last_insert_rowid(db);
}

/* expose this, because... tail recursion gets messed up when you pass function pointers. */

sqlite3_stmt* children(identifier parent) {
	DECLARE_STMT(find,"SELECT id,isdir,name,modified,modifiedns FROM entries WHERE parent = ?");
	BIND(int64)(find,1,parent);
	// this is really unreentrant... can't do that.
	return find;
}

		sqlite3_stmt* children_start(identifier parent) {
	DECLARE_STMT(find,"SELECT id,isdir,name,modified,modifiedns FROM entries WHERE parent = ?");
	s
