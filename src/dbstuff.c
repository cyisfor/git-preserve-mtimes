#include "note.h"
#include "itoa.h"
#include "dbstuff.h"
#include <stdlib.h> // NULL
#include <string.h>
#include <stdio.h>
#include <assert.h>

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


/* we can't use sqlite, because it keeps modifying the damn file...
	 may as well just use a text-based format then, ugh.
	 maybe byte #1 is a "format byte" newline for text format

	 so instead, let's be incredibly unscalable, and keep a mirror of the
	 file tree in memory, so we can add from the index, after adding
	 from the tree.

	 that means 2 copies of the entire files in memory, 1 of which we can't access
	 or modify because libgit2 sucks.
 */

static
void save_ent(int level, FILE* out, struct entry* e) {
	while(e) {
		int i;
		for(i=0;i<level;++i) {
			fputc(' ',out);
//		fputc(' ',out);
		}
		fprintf(out, "%ld.%ld %.*s\n",e->modified.tv_sec,e->modified.tv_nsec,e->namelen,e->name);
		if(e->children) {
			save_ent(level+1, out, e->children);
		}
		e = e->next;
	}
}


struct entry* load_ent(FILE* inp) {
	struct entry* cur = NULL, *root = NULL;
	char* line = NULL;
	size_t space = 0;
	int cur_level = 0;

	for(;;) {
		ssize_t amt = getline(&line, &space, inp);
		if(amt <= 0) break;
		if(line[amt-1] == '\n')
			if(--amt == 0) continue; // empty line
		
		int level;
		for(level=0;level<amt;++level) {
			if(line[level] != ' ') break;
		}

		struct entry* e = malloc(sizeof(struct entry));
		e->was_seen = false;
		struct entry* fail(const char* message) {
			WARN(message);
			free(e);
			e = root;
			while(e) {
				struct entry* next = e->next;
				free(e);
				e = next;
			}
			return NULL;
		}
		char* end = NULL;
		e->modified.tv_sec = strtol(line+level,&end,10);
		if(*end != '.')
			return fail("bad format, no dot between numbers");
		if(end == line+level)
			return fail("bad format, no seconds found");
		if(e->modified.tv_sec == 0)
			return fail("I wasn't using this in 1970.");
		e->modified.tv_nsec = strtol(end+1,&end,10);
		if(*end != ' ')
			return fail("bad format, no space after times");
		int endtime = (end - line) + 1;
		char* name = malloc(amt-endtime+1);
		memcpy(name,line+endtime,amt-endtime);
		name[amt-endtime] = '\0'; // chdir requires
		e->namelen = amt-endtime;
		e->name = name;
		e->children = NULL;

		if(cur == NULL) {
			assert(level == 0);
			e->parent = NULL;
			e->next = NULL;
			root = e;
		} else if(cur_level > level) {
			// we're moving on up
			// add this to the parent as a sibling
			assert(cur->parent);
			e->next = cur->parent->next;
			if(cur->parent->next) {
				cur->parent->next = e;
			}
			e->parent = cur->parent->parent;
			--cur_level;
		} else if(cur_level == level) {
			// add as a sibling to cur
			e->parent = cur->parent;
			e->next = cur->next;
			cur->next = e;
		} else {
			// going down. add to cur's children
			e->next = cur->children;
			cur->children = e;
			e->parent = cur;
			++cur_level;
		}
		cur = e;
	}
	return root;
}

struct entry* dbstuff_root = NULL;

struct entry* dbstuff_add(struct entry* parent,
													const char* name, int len, struct timespec mtime) {
	struct entry* e = malloc(sizeof(struct entry));
	e->was_seen = false;
	e->modified = mtime;
	char* derpname = malloc(len+1);
	memcpy(derpname,name,len);
	derpname[len] = '\0';
	e->name = derpname;
	e->namelen = len;
	if(parent == NULL) {
		if(dbstuff_root)
			e->next = dbstuff_root;
		dbstuff_root = e;
		e->parent = NULL;
	} else {
		e->parent = parent;
		e->next = parent->children;
		parent->children = e;
	}
	e->children = NULL;
	return e;
}

int dbstuff_update(struct entry* me, struct timespec mtime) {
	if(me->modified.tv_sec == mtime.tv_sec)
		if(me->modified.tv_nsec == mtime.tv_nsec)
			return 0;
	me->modified = mtime;
	return 1;
}

bool dbstuff_has_seen(struct entry* me) {
	return me->was_seen;
}

struct entry* dbstuff_find(struct entry* parent,
													 const char* name, int len) {
	struct entry* cur = parent ? parent->children : dbstuff_root;
	while(cur) {
		if(cur->namelen == len) {
			if(0==memcmp(cur->name, name, len)) {
				return cur;
			}
		}
		cur = cur->next;
	}
	return NULL;
}

const char* dbfile = NULL;

void dbstuff_close(void) {
	FILE* out = fopen(".git/temp","wt");
	save_ent(0, out, dbstuff_root);
	fclose(out);
	rename(".git/temp",dbfile);
}

void dbstuff_open(const char* dest) {
	dbfile = dest;
	FILE* inp = fopen(dbfile,"rt");
	if(inp) {
		dbstuff_root = load_ent(inp);
		fclose(inp);
	}
}
