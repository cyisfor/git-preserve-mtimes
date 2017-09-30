#define _GNU_SOURCE // st_mtim

#include "hook-common.h"

#include "ensure.h"
#include "mystring.h"
#include "repo.h"
#include "note.h"

#include <git2/revwalk.h> 
#include <git2/tree.h>
#include <git2/diff.h> 
#include <git2/refs.h>
#include <git2/commit.h>

#include <sys/stat.h>

/* we have to write our own walk, because git_tree_walk doesn't let us know
	 whether we're starting a new root, or ending one.
*/

struct treestack {
	const git_tree* tree;
	int pos;
};

static void write_entry(int out, git_tree_entry* entry) {
	const char* path = git_tree_entry_name(entry);
	struct stat info;
	ensure0(stat(path,&info));
	smallstring_write(out, path, strlen(path));
	write(out,&info.st_mtim, sizeof(info.st_mtim));
}

void store(int out, const git_tree* tree) {
	// normal recursion will stack up the oid, count, i, istree, etc.
	struct treestack* tstack = malloc(sizeof(*tstack) << 2);
	int nstack = 1;
	int sstack = 4; // space, so don't shrink
	tstack[0].tree = tree;
	tstack[0].pos = 0;
	for(;;) {
		enum operation op;
		struct treestact* ts = tstack[nstack-1];
		git_tree_entry* entry = git_tree_entry_byindex(ts->tree,ts->pos);
		if(entry == NULL) {
			op = ASCEND;
			write(out,&op,sizeof(op));
			git_tree_free(ts->tree);
			// this is the only place it could break out of the loop.
			if(--nstack == 0) break;
			continue;
		}
		bool istree = (git_tree_entry_type(entry) == GIT_OBJ_TREE);
		if(istree) {
			op = DESCEND;
		} else {
			op = ENTRY;
		}
		write(out,&op,sizeof(op));
		write_entry(out, entry);
		++ts->pos;
		if(istree) {
			git_oid* oid = git_tree_entry_id(entry);
			git_tree* tree = git_tree_lookup(oid);
			if(nstack == sstack) {
				sstack += 4;
				tstack = realloc(tstack, sizeof(*tstack)*(sstack));
			}
			tstack[nstack] = tree;
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
	store(out, head);
	rename(".tmp",TIMES_PATH);
	repo_add(TIMES_PATH);
	return 0;
}
