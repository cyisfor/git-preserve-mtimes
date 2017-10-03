#define _GNU_SOURCE

#include "db.h"

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> // write

#define PUTLIT(a) write(1,a,sizeof(a)-1)
#define PUTS(a,len) write(1,a,len)

char* escape(size_t* rlen, const char* s, size_t len) {
	size_t space = 0x100;
	char* stmt = malloc(0x100);
	size_t stmtlen = 0;
#define GROW(n) if(stmtlen + n >= space) { space += 0x100; stmt = realloc(stmt,space); }

	GROW(1);
	stmt[stmtlen++] = '"';
	
	int i;
	for(i=0;i<len;++i) {
		switch(s[i]) {
		case '"':
			GROW(2);
			stmt[stmtlen++] = '\\';
			stmt[stmtlen++] = '"';
			break;
		case '\\':
			GROW(3);
			stmt[stmtlen++] = '\\';
			stmt[stmtlen++] = '\\';
			break;
		case '\n':
			GROW(6);
			stmt[stmtlen++] = '\\';
			stmt[stmtlen++] = 'n';
			stmt[stmtlen++] = '"';
			stmt[stmtlen++] = '\n';
			stmt[stmtlen++] = '\t';
			stmt[stmtlen++] = '"';
			break;
		default:
			GROW(1);
			stmt[stmtlen++] = s[i];
		};
	}
	GROW(1);
	stmt[stmtlen++] = '"';
	
	*rlen = stmtlen;
	return stmt;
}

struct entries {
	size_t name;
	size_t namelen;
	size_t stmt;
	size_t stmtlen;
	size_t unescapedstmtlen;
};

int main(int argc, char *argv[])
{
	db_init(":memory:");
	struct stat info;
	ensure0(fstat(0,&info)); // stdin better be a file.
	char* mem = mmap(NULL,info.st_size,PROT_READ,MAP_PRIVATE,0,0);

	struct entries* entries = NULL;
	size_t nentries = 0;
	size_t sentries = 0;
	
	size_t startname = 0;

	while(startname != info.st_size) {
		while(isspace(mem[startname])) {
			if(++startname == info.st_size) {
				break; // ended in lotsa whitespace ok
			}
		}
		const char* nl = memchr(mem+startname, '\n', info.st_size-startname);
		if(nl == NULL) {
			puts("junk at the end? Should be a statement following the name!");
			return 1;
		}
		size_t startstmt = nl - mem + 1;

		while(isspace(mem[startstmt])) {
			if(++startstmt == info.st_size) {
				return 2;
			}
		}

		// then "a statement" follows.
		// only way to be sure we're at the end is to prepare it.

		const char* tail = NULL;
		sqlite3_stmt* dummy;
		db_check(sqlite3_prepare(db, mem+startstmt, info.st_size - startstmt,
														 &dummy, &tail));
		size_t endstmt = tail - mem;
		// we need to iterate 3 times over these, so just save offsets

		if(nentries == sentries) {
			sentries += 0x100;
			entries = realloc(entries,sizeof(sentries));
		}
		size_t endname = startstmt - 1;
		while(endname > startname && isspace(mem[endname])) --endname;
		entries[nentries].name = startname;
		entries[nentries].namelen = endname-startname+1;
		// parse the statement to remove weird characters

		startname = endstmt;
		while(endstmt > startstmt && isspace(mem[endstmt])) --endstmt;

		entries[nentries].unescapedstmtlen = endstmt - startstmt + 1;
		
		entries[nentries].stmt = escape(&entries[nentries].stmtlen,
																		mem+startstmt, endstmt-startstmt+1);
		
		++nentries;
	}

	// now some C

	// sqlite3_stmt* name;
	int i;
	for(i=0;i<nentries;++i) {
		PUTLIT("sqlite3_stmt* ");
		PUTS(mem + entries[i].name, entries[i].namelen);
		PUTLIT(";\n");
	}
	PUTLIT("\nvoid prepare_init(void) {\n");

	// 	db_check(sqlite3_prepare(db,%2$,%3$, &%1$, NULL));
	for(i=0;i<nentries;++i) {
		PUTLIT("\tdb_check(sqlite3_prepare(db,");
		PUTS(entries[i].stmt,entries[i].stmtlen); // includes "'s
		PUTLIT(", ");
		char buf[0x100];
		PUTS(buf,	atoi(buf,0x100,entries[i].unescapedstmtlen)); 
		PUTLIT(", &");
		PUTS(entries[i].name, entries[i].namelen);
		PUTLIT(", NULL);\n");
	}
	PUTLIT("}\n\n");

	PUTLIT("void prepare_finalize(void) {\n");
	for(i=0;i<nentries;++i) {
		PUTLIT("\tdb_check(sqlite3_finalize(");
		PUTS(entries[i].name, entries[i].namelen);
		PUTLIT(");\n");
	}
	PUTLIT("}\n");
	return 0;
}
