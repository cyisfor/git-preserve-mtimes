#define _GNU_SOURCE // st_mtim

#include "hook-common.h"
#include "smallstring.h"

#include "ensure.h"
#include "mystring.h"
#include "repo.h"
#include "note.h"

#include <git2/revwalk.h> 
#include <git2/tree.h>
#include <git2/diff.h> 
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

static void write_entry(int out, const git_tree_entry* entry) {
	INFO("writing %s",path);
	struct stat info;
	ensure0(stat(path,&info));
	smallstring_write(out, path, strlen(path));
	write(out,&info.st_mtim, sizeof(info.st_mtim));
	return ret;
}

void store(int out, git_tree* tree) {
	// normal recursion will stack up the oid, count, i, istree, etc.
	struct treestack* tstack = malloc(sizeof(*tstack) << 2);
	int nstack = 1;
	int sstack = 4; // space, so don't shrink
	tstack[0].tree = tree;
	tstack[0].pos = 0;
	tstack[0].nlen = 0;

	// since we have to also add the index, we need to check if stuff has been seen...

	struct strings* seen_paths = NULL;
	uint32_t last_seen = 1;

	char* root = NULL;
	size_t rootlen = 0;
	size_t rootspace = 0;
		
	for(;;) {
		enum operation op;
		struct treestack* ts = &tstack[nstack-1];
		const git_tree_entry* entry = git_tree_entry_byindex(ts->tree,ts->pos);
		if(entry == NULL) {
			op = ASCEND;
			write(out,&op,sizeof(op));
			// a/b/c/d -> a/b/c
			rootlen -= ts->nlen + 1;
			git_tree_free(ts->tree);
			// this is the only place it could break out of the loop.
			INFO("ascending");
			if(--nstack == 0) break;
			chdir("..");
			continue;
		}
		bool istree = (git_tree_entry_type(entry) == GIT_OBJ_TREE);
		if(istree) {
			op = DESCEND;
		} else {
			op = ENTRY;
		}
		write(out,&op,sizeof(op));
		string name;
		{
			name.s = git_tree_entry_name(entry);
			name.l = strlen(name.s);
			if(rootspace - rootlen < name.l + 2) {
				rootspace = rootlen + name.l + 2;
				root = realloc(root,rootspace);
			}
			root[rootlen] = '/';
			memcpy(root+rootlen+1,name.s,name.l);
			root[rootlen+name.l+2] = '\0';
			uint32_t seen = strings_intern(seen_paths, root);
			if(seen < last_seen) {
				// already interned, so this entry's already been written
				continue;
			}
		}
		write_entry(out, entry);
		++ts->pos;
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
			if(rootsize - rootlen > name.l + 1) {
				rootsize = rootlen + name.l + 1;
				root = realloc(root,rootsize);
			}
			root[rootlen] = '/';
			memcpy(root+rootlen+1,name.s,name.l);
			rootlen += name.l + 1;
			// no need for \0 terminator yet
			INFO("descending");
			ensure0(chdir(path));
			++nstack;
		}
	}
	free(tstack);
}


int main(int argc, char *argv[])
{
	repo_discover_init(".",1);
	// traverse head, storing mtimes in .git_times, adding .git_times to the repo
	git_tree* head;
	{
		// since HEAD
		git_reference* ref;
		repo_check(git_repository_head(&ref, repo)); // this resolves th
		const git_oid * oid = git_reference_target(ref);
		git_commit* commit;
		repo_check(git_commit_lookup(&commit, repo, oid));
		repo_check(git_commit_tree(&head,commit));
		git_commit_free(commit);
	}

	int out = open(".tmp",O_WRONLY|O_CREAT|O_TRUNC,0644);
	ensure_ge(out,0);
	store(out, head);
	rename(".tmp",TIMES_PATH);
	system("pwd");
	repo_add(TIMES_PATH);
	return 0;
}
