#ifndef _DBSTUFF_H_
#define _DBSTUFF_H_

#include <stdbool.h>
#include <time.h> // timespec

struct entry {
	struct entry* children;
	struct entry* parent;
	struct entry* next;
	const char* name;
	int namelen;
	struct timespec modified;
	bool was_seen;
};

extern
struct entry* dbstuff_root;

struct entry* dbstuff_find(struct entry* parent,
													 const char* name, int len);

int dbstuff_update(struct entry* me, struct timespec mtime);

struct entry* dbstuff_add(struct entry* parent,
													const char* name, int len, struct timespec mtime);

void dbstuff_open(const char*);
void dbstuff_close(void);


#endif /* _DBSTUFF_H_ */
