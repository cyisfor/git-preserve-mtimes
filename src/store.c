#define _GNU_SOURCE // st_mtim

#include "hook-common.h"
#include "dbstuff.h"
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

bool dirty = false;

static identifier write_entry(int out, identifier parent, const string name, bool istree) {
	struct stat info;
	if(0 != stat(name.s,&info)) {
		//INFO("deleted %.*s",name.l,name.s);
		// allow this marked as "seen" since we don't need to save it at all.
		return -1;
	}
	dirty = true;
	return dbstuff_add(parent,name.s,name.l,istree, info.st_mtim);
}

/* this is stupidified because libgit2 hides its interface too well.
	 since we can't walk a treebuilder, to walk the trees in it, without
	 recursion, 
	 tree/treebuilder have different interfaces
	 so the topmost tree has to be a treebuilder, since we merged the index
	 into it. But every tree below it, we can just walk like a normal tree.
*/

void store_tree(int out, identifier parent, git_tree* tree) {
	int pos;
	int num = git_tree_entrycount(tree);
	for(pos=0;pos<num;++pos) {
		const git_tree_entry* entry = git_tree_entry_byindex(tree,pos);
		if(entry == NULL) {
			return;
		}

		string name;
		{
			name.s = git_tree_entry_name(entry);
			name.l = strlen(name.s);

			if(name.l == sizeof(TIMES_PATH) &&
				 0 == memcmp(name.s, TIMES_PATH, name.l))
				continue;

			if(dbstuff_has(parent, name.s, name.l)) {
				continue;
			}
		}

		int type = git_tree_entry_type(entry);
		bool istree = (type == GIT_OBJ_TREE);
		identifier me = write_entry(out, parent, name, istree);
		if(istree) {
			const git_oid* oid = git_tree_entry_id(entry);
			git_tree* tree;
			repo_check(git_tree_lookup(&tree, repo, oid));
			chdir(name.s);
			store_tree(out, me, tree);
			chdir("..");
			git_tree_free(tree);
		}
	}
}

void store_index(int out) {
	git_index* index = NULL;
	repo_check(git_repository_index(&index, repo));
	//INFO("index %s",git_index_path(index));
	size_t count = git_index_entrycount(index);
	size_t i;
	for(i=0;i<count;++i) {
		const git_index_entry * entry = git_index_get_byindex(index,i);
		ensure_ne(entry,NULL);

		void onelevel(identifier parent, const char* path, size_t len) {
			if(!len) return;
			const char* slash = memchr(path,'/',len);
			size_t clen;
			bool istree;
			if(slash == NULL) {
				clen = len;
				istree = false;
			} else {
				clen = slash - path;
				istree = true;
			}

			const string name = {
				.s = path,
				.l = clen
			};
			
			if(name.l == sizeof(TIMES_PATH) &&
				 0 == memcmp(name.s, TIMES_PATH, name.l))
				return;
			if(dbstuff_has(parent, name.s, name.l)) {
				return;
			}
			identifier me = write_entry(out, parent, name, istree);
			if(istree) {
				onelevel(me, path+clen+1, len-clen-1);
			}
		}

		const char* path = git_index_path(index);
		onelevel(0, path, strlen(path));
	}
	git_index_free(index);
}

int main(int argc, char *argv[])
{
	note_init();
	repo_discover_init(".",1);
	db_init(TIMES_PATH);
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
			store_tree(out, 0, head);
		store_index(out);
		if(dirty) {
			rename(temp,TIMES_PATH);
			repo_add(TIMES_PATH);
		} else {
			unlink(temp);
		}
		close(out);
	}
	return 0;
}
