#define _GNU_SOURCE // futimens, st_mtim
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

enum { DESCEND, ENTRY, ASCEND };

typedef int on_entry(const char *root, const git_tree_entry *entry, void *payload) {
	int out = (int) ((intptr_t) payload);
	const char* name = git_tree_entry_name(entry);
	smallstring_write(out,s,strlen(s));
	



int main(int argc, char *argv[])
{
	repo_discover_init(".",1);
	// follow git index, storing mtimes in .gittime, adding .gittime to the repo
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

	

	return 0;
}
