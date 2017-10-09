#include "itoa.h"
#include "dbstuff.h"
#include <stdlib.h> // NULL
#include <string.h>

#define LITLEN(a) a,(sizeof(a)-1)

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

struct entry {
	struct entry* children;
	int nkids;
	struct entry* parent;
	struct entry* next;
	const char* name;
	struct timespec modified;
};

/* we can't use sqlite, because it keeps modifying the damn file...
	 may as well just use a text-based format then, ugh.
	 maybe byte #1 is a "format byte" newline for text format

	 so instead, let's be incredibly unscalable, and keep a mirror of the
	 file tree in memory, so we can add from the index, after adding
	 from the tree.

	 that means 2 copies of the entire files in memory, 1 of which we can't access
	 or modify because libgit2 sucks.
 */

struct entry* root = NULL;

void save(int level, FILE* out, struct entry* e) {
	int i;
	for(i=0;i<level;++i) {
		fputc(' ',out);
//		fputc(' ',out);
	}
	fprintf(out, "%ld.%ld %s\n",e->modified.tv_sec,e->modified.tv_nsec,e->name);
	if(e->nkids) {
		int i = 0;
		for(i=0;i<e->nkids;++i) {
			serialize(level+1, out, &e->children[i]);
		}
	}
}


void load(int old_level, struct entry* parent, FILE* inp) {
	static char* line = NULL;
	static size_t space = 0;

	ssize_t amt = getline(&line, &space, inp);
	if(amt <= 0) return NULL;

	int level;
	for(level=0;level<amt;++level) {
		if(line[level] != ' ') break;
	}

	struct entry* e = malloc(sizeof(struct entry));
	int endtime = sscanf(line+level,amt-level,"%ld.%ld ",
											 &e->modified.tv_sec,
											 &e->modified.tv_nsec);

	char* name = malloc(amt-endtime-level);
	memcpy(name,line+level+endtime,amt-endtime-level);

	e->name = name;

	if(old_level > level) {
		// we're moving on up
		// add this to the parent's parent, then return.
		// this is never more than 1 level different
		assert(parent);
		parent = parent->parent;
		++parent->nkids;
		parent->children = realloc(parent->children,sizeof(*parent->children)*parent->nkids);
		parent->children[nkids-1] = e;
		return e;
	}
	if(old_level == level) {
		
		
	
	e->children = NULL;
	e->nkids = 0;
	e->
	
	int i;
	for(i=0;i<level;++i) {
		fputc(' ',out);
		fputc(' ',out);
	}
	fprintf(out, "%ld.%ld\t%s\n",e->modified.tv_sec,e->modified.tv_nsec,e->name);
	if(e->nkids) {
		int i = 0;
		for(i=0;i<e->nkids;++i) {
			serialize(level+1, out, &e->children[i]);
		}
	}
}


struct entry* dbstuff_add(struct entry* parent,
													const char* name, int len, bool isdir, struct timespec mtime) {
	BIND(text)(add_insert, 1, name, len, NULL);
	BIND(int64)(add_insert, 2, parent);
	BIND(int)(add_insert, 3, isdir ? 1 : 0);
	BIND(int64)(add_insert, 4, mtime.tv_sec);
	BIND(int64)(add_insert, 5, mtime.tv_nsec);
	STEP(add_insert);
	sqlite3_reset(add_insert);
	return sqlite3_last_insert_rowid(db);
}

int dbstuff_update(identifier me, bool isdir, struct timespec mtime) {
	BIND(int)(update, 1, isdir ? 1 : 0);
	BIND(int64)(update, 2, mtime.tv_sec);
	BIND(int64)(update, 3, mtime.tv_nsec);
	BIND(int64)(update, 4, me);
	STEP(update);
	sqlite3_reset(update);
	return sqlite3_changes(db);
}

bool dbstuff_has_seen(identifier me) {
	if(me == -1) return false;
	// but maybe haven't seen this session
	BIND(int64)(has_seen, 1, me);
	int res = STEP(has_seen);
	sqlite3_reset(has_seen);
	if(res == SQLITE_ROW) return true;
	BIND(int64)(see_entry,1,me);
	STEP(see_entry);
	sqlite3_reset(see_entry);
	return false;
}

identifier dbstuff_find(identifier parent,
												const char* name, int len) {
	BIND(text)(add_find, 1, name, len, NULL);
	BIND(int64)(add_find, 2, parent);
	int res = STEP(add_find);
	if(res == SQLITE_ROW) {
		identifier ret = sqlite3_column_int64(add_find, 0);
		sqlite3_reset(add_find);
		return ret;
	}
	sqlite3_reset(add_find);
	return -1;
}

/* expose this, because... tail recursion gets messed up when you pass function pointers. */

sqlite3_stmt* dbstuff_children(identifier parent) {
	// this must be reentrant! we will call children, before resetting children, when recursing!
	sqlite3_stmt* stmt;
#define PREFIX "CREATE TEMPORARY TABLE IF NOT EXISTS children"
	char buf[0x100] = PREFIX;
	size_t end = sizeof(PREFIX)-1 +
		itoa(buf+sizeof(PREFIX)-1,0x100-sizeof(PREFIX)+1, parent);
	memcpy(buf+end,LITLEN(" AS SELECT id,isdir,name,modified,modifiedns FROM entries WHERE parent = ? AND id != 0"));
	PREPARE(stmt,buf);
	BIND(int64)(stmt,1,parent);
	STEP(stmt); // if not created, then okay.
	sqlite3_finalize(stmt);
	// now get results
#undef PREFIX
#define PREFIX "SELECT * FROM children"
	memcpy(buf,LITLEN(PREFIX));
	end = sizeof(PREFIX)-1 +
		itoa(buf+sizeof(PREFIX)-1,0x100-sizeof(PREFIX)+1,parent);
	buf[end] = '\0';
	PREPARE(stmt,buf);
	return stmt;
#undef PREFIX
}

// note: children(0) -> topmost entries

void dbstuff_close(void) {
	prepare_finalize();
	db_close();
}
