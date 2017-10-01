#define _GNU_SOURCE // st_mtim

#include "intern/strings.h"
#include "hook-common.h"
#include "smallstring.h"

#include "ensure.h"
#include "mystring.h"
#include "repo.h"
#include "note.h"

#include <git2/tree.h>
#include <git2/index.h>
#include <git2/refs.h>
#include <git2/commit.h>

#include <unistd.h> // write
#include <fcntl.h> // open, O_*
#include <stdio.h> // rename

#include <sys/stat.h>

/* we have to write our own walk, because git_tree_walk doesn't let us know
	 whether we're starting a new root, or ending one.
*/

struct treestack {
	git_tree* tree;
	int pos;
	int nlen;
};

static void write_entry(int out, const string name) {
	INFO("writing %.*s",name.l,name.s);
	struct stat info;
	ensure0(stat(name.s,&info));
	smallstring_write(out, name.s, name.l);
	write(out,&info.st_mtim, sizeof(info.st_mtim));
}


// since we have to also add the index, we need to check if stuff has been seen...

struct strings* seen_paths = NULL;
uint32_t last_seen = 1;

char* root = NULL;
size_t rootlen = 0;
size_t rootspace = 0;

bool has_seen(string name) {
	// never write anything named .git_times
	if(name.l == LITSIZ(TIMES_PATH) &&
		 0==memcmp(name.s,LITLEN(TIMES_PATH)))
		return true;
	if(rootspace - rootlen < name.l + 2) {
		rootspace = rootlen + name.l + 2;
		root = realloc(root,rootspace);
	}
	size_t len = rootlen;
	if(len > 0) {
		root[len++] = '/';
	}
	memcpy(root+len,name.s,name.l);
	len += name.l;
	root[len] = '\0';
	// now root is the current full path of name
	uint32_t seen = strings_intern(seen_paths, root);
	bool ret = seen < last_seen;
	// < = already interned, so this entry's already been written
	INFO("has_seen %.*s %d < %d %d",len,root,seen, last_seen, ret);
	if(!ret) {
		last_seen = seen;
	}
	return ret;
}

void extend_root(string name) {
	if(rootspace - rootlen > name.l + 1) {
		rootspace = rootlen + name.l + 1;
		root = realloc(root,rootspace);
	}
	if(rootlen > 0) {
		root[rootlen++] = '/';
	}
	memcpy(root+rootlen,name.s,name.l);
	rootlen += name.l;
	// no need for \0 terminator yet
}

void store(int out, git_tree* tree) {
	// normal recursion will stack up the oid, count, i, istree, etc.
	struct treestack* tstack = malloc(sizeof(*tstack) << 2);
	int nstack = 1;
	int sstack = 4; // space, so don't shrink
	tstack[0].tree = tree;
	tstack[0].pos = 0;
	tstack[0].nlen = 0;
		
	for(;;) {
		enum operation op;
		struct treestack* ts = &tstack[nstack-1];
		const git_tree_entry* entry = git_tree_entry_byindex(ts->tree,ts->pos);
		++ts->pos;

		if(entry == NULL) {
			if(nstack == 1) {
				// we ascended to the top
				break;
			}
			op = ASCEND;
			write(out,&op,sizeof(op));
			// a/b/c/d -> a/b/c
			// unextend root?
			ensure_ge(rootlen,ts->nlen);
			rootlen -= ts->nlen;
			if(rootlen > 0) --rootlen; // eat /
			git_tree_free(ts->tree);
			// this is the only place it could break out of the loop.
			INFO("ascending");
			ensure_gt(--nstack, 0);
			chdir("..");
			continue;
		}

		string name;
		{
			name.s = git_tree_entry_name(entry);
			name.l = strlen(name.s);
			if(has_seen(name)) {
				continue;
			}
		}
		
		bool istree = (git_tree_entry_type(entry) == GIT_OBJ_TREE);
		if(istree) {
			op = DESCEND;
		} else {
			op = ENTRY;
		}
		write(out,&op,sizeof(op));
		write_entry(out, name);
		if(istree) {
			const git_oid* oid = git_tree_entry_id(entry);
			git_tree* tree;
			repo_check(git_tree_lookup(&tree, repo, oid));
			if(nstack == sstack) {
				sstack += 4;
				tstack = realloc(tstack, sizeof(*tstack)*(sstack));
			}
			tstack[nstack].tree = tree;
			tstack[nstack].pos = 0;
			tstack[nstack].nlen = name.l;
			extend_root(name);
			INFO("descending");
			ensure0(chdir(name.s));
			++nstack;
		}
	}
	free(tstack);
}

void store_index(int out) {
		enum operation op = ENTRY;
		git_index* index;
		git_repository_index(&index, repo);
		size_t count = git_index_entrycount(index);
		size_t i;
		for(i=0;i<count;++i) {
			const git_index_entry * entry = git_index_get_byindex(index,i);
			ensure_ne(entry,NULL);
			string path = {
				.s = entry->path,
				.l = strlen(entry->path)
			};
			if(has_seen(path)) {
				continue;
			}
			// path has / in it, but there's no requirement for entries not to.
			write(out,&op,sizeof(op));
			write_entry(out,path);
		}
		git_index_free(index);
	}
}


int main(int argc, char *argv[])
{
	seen_paths = strings_new();
	repo_discover_init(".",1);
	// traverse head, storing mtimes in .git_times, adding .git_times to the repo
	git_tree* head = NULL;
	{
		// since HEAD
		git_reference* ref;
		int res = git_repository_head(&ref, repo); // this resolves
		if(res == GIT_OK) {
			const git_oid * oid = git_reference_target(ref);
			git_commit* commit;
			repo_check(git_commit_lookup(&commit, repo, oid));
			repo_check(git_commit_tree(&head,commit));
			git_commit_free(commit);
		}
		// otherwise there is no head (initial commit)
		// only index
	}

	char temp[] = ".tmpXXXXXX";

	int out = mkstemp(temp);
	if(note_catch()) {
		unlink(temp);
		abort();
	} else {
		ensure_ge(out,0);
		if(head)
			store(out, head);
		store_index(out);
		rename(temp,TIMES_PATH);
		system("pwd");
		repo_add(TIMES_PATH);
	}
	return 0;
}
